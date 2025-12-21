```javascript
const assert = require('assert');
const fs = require('fs');
const path = require('path');
const { EventEmitter } = require('events');

// Mock dependencies
const mockFs = require('mock-fs');
const nock = require('nock');

// Import functions to test
const {
  loadConfig,
  validateModelConfig,
  fetchAvailableModels,
  extractTokenUsage,
  calculateCosts,
  generateModelsForProvider,
  writeAccountingEntry
} = require('./llm-proxy');

describe('LLM Proxy Configuration', () => {
  beforeEach(() => {
    // Reset mock filesystem before each test
    mockFs({
      'llm-proxy-config.json': JSON.stringify({
        providers: {
          openai: {
            apiKey: 'sk-test123',
            models: [{
              id: 'gpt-4',
              contextWindow: 8192,
              pricing: {
                input: 30.0,
                output: 60.0,
                cacheRead: 15.0
              }
            }]
          }
        }
      })
    });
  });

  afterEach(() => {
    // Restore real filesystem
    mockFs.restore();
    // Clear any nock interceptors
    nock.cleanAll();
  });

  describe('loadConfig', () => {
    it('should load configuration from file', () => {
      const config = loadConfig();
      assert.ok(config.providers.openai, 'OpenAI provider should be present');
      assert.strictEqual(config.providers.openai.apiKey, 'sk-test123', 'API key should match');
    });

    it('should handle missing configuration file', () => {
      // Remove config file to simulate first-time setup
      mockFs({});
      
      try {
        loadConfig();
        assert.fail('Should have thrown an error');
      } catch (error) {
        assert.ok(error, 'Error should be thrown for missing config');
      }
    });
  });

  describe('validateModelConfig', () => {
    it('should validate OpenAI model configuration', () => {
      const validModel = {
        id: 'gpt-4',
        contextWindow: 8192,
        pricing: {
          input: 30.0,
          output: 60.0,
          cacheRead: 15.0
        }
      };

      const result = validateModelConfig('openai', validModel, 'openai');
      assert.strictEqual(result, null, 'Valid model should return null');
    });

    it('should reject invalid model configurations', () => {
      const invalidModels = [
        // Missing ID
        {
          contextWindow: 8192,
          pricing: { input: 30.0, output: 60.0 }
        },
        // Negative context window
        {
          id: 'test-model',
          contextWindow: -100,
          pricing: { input: 30.0, output: 60.0 }
        },
        // Missing pricing
        {
          id: 'test-model',
          contextWindow: 8192
        }
      ];

      invalidModels.forEach(model => {
        const result = validateModelConfig('openai', model, 'openai');
        assert.notStrictEqual(result, null, `Should reject model: ${JSON.stringify(model)}`);
      });
    });
  });

  describe('fetchAvailableModels', () => {
    it('should fetch models for OpenAI', async () => {
      // Setup nock to mock OpenAI API response
      nock('https://api.openai.com')
        .get('/v1/models')
        .reply(200, {
          data: [
            { id: 'gpt-4' },
            { id: 'gpt-3.5-turbo' }
          ]
        });

      const models = await fetchAvailableModels('openai', 'sk-test123', { baseUrl: 'https://api.openai.com' });
      
      assert.ok(models, 'Models should be returned');
      assert.ok(models.length > 0, 'At least one model should be found');
      assert.ok(models.some(m => m.id === 'gpt-4'), 'GPT-4 model should be present');
    });

    it('should handle API errors when fetching models', async () => {
      // Setup nock to simulate API error
      nock('https://api.openai.com')
        .get('/v1/models')
        .reply(401, { error: 'Unauthorized' });

      const models = await fetchAvailableModels('openai', 'invalid-key', { baseUrl: 'https://api.openai.com' });
      
      assert.strictEqual(models, null, 'Should return null on API error');
    });
  });

  describe('extractTokenUsage', () => {
    it('should extract token usage for OpenAI', () => {
      const openAIResponse = {
        usage: {
          prompt_tokens: 100,
          completion_tokens: 50,
          prompt_tokens_details: {
            cached_tokens: 25
          }
        }
      };

      const usage = extractTokenUsage('openai', openAIResponse);
      
      assert.strictEqual(usage.promptTokens, 100, 'Prompt tokens should be extracted');
      assert.strictEqual(usage.completionTokens, 50, 'Completion tokens should be extracted');
      assert.strictEqual(usage.cachedTokens, 25, 'Cached tokens should be extracted');
    });

    it('should handle missing usage data', () => {
      const emptyResponse = {};
      
      const usage = extractTokenUsage('openai', emptyResponse);
      
      assert.strictEqual(usage.promptTokens, 0, 'Should default to 0 for missing prompt tokens');
      assert.strictEqual(usage.completionTokens, 0, 'Should default to 0 for missing completion tokens');
    });
  });

  describe('calculateCosts', () => {
    it('should calculate token costs correctly', () => {
      const tokens = {
        promptTokens: 1000,
        completionTokens: 500,
        cachedTokens: 200
      };

      const pricing = {
        input: 10.0,   // $10 per million input tokens
        output: 30.0,  // $30 per million output tokens
        cacheRead: 5.0 // $5 per million cached tokens
      };

      const costs = calculateCosts(tokens, pricing);
      
      // Expected costs:
      // Prompt: (1000 * 10) / 1,000,000 = $0.01
      // Completion: (500 * 30) / 1,000,000 = $0.015
      // Cache Read: (200 * 5) / 1,000,000 = $0.001
      assert.strictEqual(costs.inputCost, 0.01, 'Input cost should be correct');
      assert.strictEqual(costs.outputCost, 0.015, 'Output cost should be correct');
      assert.strictEqual(costs.cacheReadCost, 0.001, 'Cache read cost should be correct');
    });

    it('should handle missing pricing', () => {
      const tokens = {
        promptTokens: 1000,
        completionTokens: 500
      };

      const costs = calculateCosts(tokens, null);
      
      assert.strictEqual(costs.inputCost, 0, 'Input cost should be 0');
      assert.strictEqual(costs.outputCost, 0, 'Output cost should be 0');
    });
  });

  describe('generateModelsForProvider', () => {
    it('should generate models for OpenAI', () => {
      const openAIModels = generateModelsForProvider('openai');
      
      assert.ok(openAIModels.length > 0, 'Should return at least one model');
      assert.ok(openAIModels.some(m => m.id.startsWith('gpt')), 'Should include GPT models');
    });

    it('should generate models for Anthropic', () => {
      const anthropicModels = generateModelsForProvider('anthropic');
      
      assert.ok(anthropicModels.length > 0, 'Should return at least one model');
      assert.ok(anthropicModels.some(m => m.id.startsWith('claude')), 'Should include Claude models');
    });
  });

  describe('writeAccountingEntry', () => {
    const accountingDir = path.join(process.cwd(), 'logs');
    const accountingFile = path.join(accountingDir, `llm-accounting-${new Date().toISOString().split('T')[0]}.jsonl`);
    
    beforeEach(() => {
      // Ensure accounting directory exists
      if (!fs.existsSync(accountingDir)) {
        fs.mkdirSync(accountingDir, { recursive: true });
      }
    });

    it('should write accounting entry to log file', () => {
      const testEntry = {
        timestamp: new Date().toISOString(),
        provider: 'openai',
        model: 'gpt-4',
        tokens: {
          prompt: 100,
          completion: 50
        },
        costs: {
          input: 0.01,
          output: 0.015
        }
      };

      // Spy on console.error to check error handling
      const originalConsoleError = console.error;
      const errorSpy = jest.fn();
      console.error = errorSpy;

      try {
        writeAccountingEntry(testEntry);

        // Read the log file
        const logContents = fs.readFileSync(accountingFile, 'utf8');
        const lastLine = logContents.trim().split('\n').pop();
        const parsedEntry = JSON.parse(lastLine);

        assert.deepStrictEqual(parsedEntry, testEntry, 'Accounting entry should be written correctly');
        assert.strictEqual(errorSpy.mock.calls.length, 0, 'No errors should be logged');
      } finally {
        // Restore console.error
        console.error = originalConsoleError;
      }
    });

    it('should handle errors when writing accounting entry', () => {
      // Make the directory read-only to force an error
      fs.chmodSync(accountingDir, 0o444);

      const testEntry = {
        timestamp: new Date().toISOString(),
        provider: 'test',
        model: 'error-model'
      };

      // Spy on console.error to verify error logging
      const originalConsoleError = console.error;
      const errorSpy = jest.fn();
      console.error = errorSpy;

      try {
        writeAccountingEntry(testEntry);

        // Check that console.error was called
        assert.ok(errorSpy.mock.calls.length > 0, 'Should log an error when writing fails');
        assert.ok(errorSpy.mock.calls[0][0].includes('Failed to write accounting entry'), 'Error message should indicate write failure');
      } finally {
        // Restore directory permissions and console.error
        fs.chmodSync(accountingDir, 0o755);
        console.error = originalConsoleError;
      }
    });
  });
});
```