#pragma once

#ifndef NDEBUG
// 複数値の結合には << でなく + か Format() を使う: APP_LOG(U"n=" + Format(n))
#define APP_LOG(msg) Console << (msg)
#else
#define APP_LOG(msg) static_cast<void>(0)
#endif
