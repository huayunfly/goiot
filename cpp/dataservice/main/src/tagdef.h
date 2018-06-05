//------------------------------------------------------------
// @File: <tagdef.h>
// @Purpose: Tag definition
//
// @Author: <Yun Hua>
// @Version: 0.1 2018/06/05
//
// Copyright (C) 2018, Yun Hua
//------------------------------------------------------------

#include <variant>

typedef double GO_TYPE_REAL;
typedef int GO_TYPE_INT;
typedef long int GO_TYPE_LONG; 

struct goTagValue
{
    std::variant<GO_TYPE_INT, bool, GO_TYPE_REAL> value;
};
