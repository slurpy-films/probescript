// probescript.h
// This file declares the functions in probescript's C API

#ifndef PROBESCRIPT_H
#define PROBESCRIPT_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct Prb_VM Prb_VM;
typedef struct Prb_Compiler Prb_Compiler;

typedef Prb_Value* (*Prb_CFunction)(int argc, Prb_Value **argv);

typedef enum
{
    Prb_String,
    Prb_Number,
    Prb_Null,
    Prb_Function,
} Prb_ValueType;

typedef struct
{
    Prb_ValueType type;

    union
    {
        char *string;
        double number;
        Prb_CFunction function;
    } as;
} Prb_Value;

Prb_VM *prb_create_vm(void);
Prb_Compiler *prb_create_compiler(const char *filename);

void prb_destroy_vm(Prb_VM *vm);
void prb_destroy_compiler(Prb_Compiler *compiler);

void prb_run(Prb_VM *vm, Prb_Compiler *compiler);

void prb_register_fn(Prb_VM *vm, const char *name, Prb_CFunction fn);

Prb_Value *prb_lookup(Prb_VM *vm, const char *name);

#ifdef __cplusplus
} // extern "C"
#endif

#endif