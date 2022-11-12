/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include <new>

#ifdef _DEBUG
void *operator new(size_t size);

void operator delete(void *ptr) noexcept;
#endif
