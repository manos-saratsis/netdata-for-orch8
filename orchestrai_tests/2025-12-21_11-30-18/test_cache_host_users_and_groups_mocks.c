// Mocking functions to control behavior in unit tests
int __wrap_spinlock_trylock(SPINLOCK *spinlock) {
    return mock_spinlock_trylock_result;
}

procfile *__wrap_procfile_open(const char *filename, const char *separators, int flags) {
    if (mock_procfile_open_result) 
        return (procfile *)1;  // Return non-NULL to simulate successful open
    return NULL;
}

procfile *__wrap_procfile_readall(procfile *pf) {
    if (mock_procfile_readall_result)
        return pf;
    return NULL;
}

size_t __wrap_procfile_lines(procfile *pf) {
    return mock_procfile_lines_result;
}

size_t __wrap_procfile_linewords(procfile *pf, size_t line) {
    return mock_procfile_linewords_result;
}

char *__wrap_procfile_lineword(procfile *pf, size_t line, size_t word) {
    return mock_procfile_lineword_result;
}

void __wrap_procfile_close(procfile *pf) {
    // No-op for test
}