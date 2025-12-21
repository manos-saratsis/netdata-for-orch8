import io
import pytest
from pyyaml3.emitter import Emitter, EmitterError, ScalarAnalysis
from pyyaml3.events import (
    StreamStartEvent, DocumentStartEvent, ScalarEvent, 
    StreamEndEvent, DocumentEndEvent
)

class TestEmitterBasics:
    def test_init_with_default_parameters(self):
        stream = io.StringIO()
        emitter = Emitter(stream)
        assert emitter is not None
        assert emitter.stream == stream

    def test_init_with_full_parameters(self):
        stream = io.StringIO()
        emitter = Emitter(stream, 
            canonical=True, 
            indent=4, 
            width=120,
            allow_unicode=True, 
            line_break='\r\n'
        )
        assert emitter is not None
        assert emitter.canonical is True
        assert emitter.best_indent == 4
        assert emitter.best_width == 120
        assert emitter.allow_unicode is True
        assert emitter.best_line_break == '\r\n'

    def test_dispose(self):
        stream = io.StringIO()
        emitter = Emitter(stream)
        emitter.dispose()
        assert emitter.states == []
        assert emitter.state is None

class TestEmitterEvents:
    def test_emit_stream_start(self):
        stream = io.StringIO()
        emitter = Emitter(stream)
        start_event = StreamStartEvent()
        emitter.emit(start_event)
        # Test state transition and basic event handling
        assert emitter.state.__name__ == 'expect_first_document_start'

    def test_emit_sequence_of_events(self):
        stream = io.StringIO()
        emitter = Emitter(stream)
        events = [
            StreamStartEvent(),
            DocumentStartEvent(),
            ScalarEvent(None, None, ('', ''), 'test', style=None, implicit=(True, True)),
            DocumentEndEvent(explicit=False),
            StreamEndEvent()
        ]
        for event in events:
            emitter.emit(event)
        # Minimal smoke test to ensure no exceptions
        assert len(emitter.events) == 0

class TestEmitterScalarAnalysis:
    def test_analyze_empty_scalar(self):
        stream = io.StringIO()
        emitter = Emitter(stream)
        analysis = emitter.analyze_scalar('')
        assert analysis.empty is True
        assert analysis.allow_block_plain is True
        assert analysis.allow_single_quoted is True
        assert analysis.allow_double_quoted is True

    def test_analyze_simple_scalar(self):
        stream = io.StringIO()
        emitter = Emitter(stream)
        analysis = emitter.analyze_scalar('hello world')
        assert analysis.empty is False
        assert analysis.multiline is False
        assert analysis.allow_flow_plain is True

    def test_analyze_multiline_scalar(self):
        stream = io.StringIO()
        emitter = Emitter(stream)
        analysis = emitter.analyze_scalar('hello\nworld')
        assert analysis.multiline is True
        assert analysis.allow_flow_plain is False
        assert analysis.allow_block_plain is False

class TestEmitterErrorHandling:
    def test_emit_without_stream_start_raises_error(self):
        stream = io.StringIO()
        emitter = Emitter(stream)
        with pytest.raises(EmitterError):
            emitter.emit(DocumentStartEvent())

class TestEmitterTagAndAnchorProcessing:
    def test_prepare_tag_handle(self):
        stream = io.StringIO()
        emitter = Emitter(stream)
        handle = emitter.prepare_tag_handle('!my_tag!')
        assert handle == '!my_tag!'

    def test_invalid_tag_handle_raises_error(self):
        stream = io.StringIO()
        emitter = Emitter(stream)
        with pytest.raises(EmitterError):
            emitter.prepare_tag_handle('invalid')

class TestEmitterSpecialWriters:
    def test_write_single_quoted(self):
        stream = io.StringIO()
        emitter = Emitter(stream)
        emitter.write_single_quoted('hello world')
        result = stream.getvalue()
        assert result.startswith("'")
        assert result.endswith("'")

    def test_write_double_quoted(self):
        stream = io.StringIO()
        emitter = Emitter(stream)
        emitter.write_double_quoted('hello world')
        result = stream.getvalue()
        assert result.startswith('"')
        assert result.endswith('"')

class TestEmitterEdgeCases:
    def test_unicode_scalar_analysis(self):
        stream = io.StringIO()
        emitter = Emitter(stream, allow_unicode=False)
        analysis = emitter.analyze_scalar('こんにちは')
        assert analysis.special_characters is True

    def test_line_break_handling(self):
        stream = io.StringIO()
        emitter = Emitter(stream, line_break='\r\n')
        assert emitter.best_line_break == '\r\n'