#include "dyncall_any.h"
#include "../types/all_cb_types.h"

namespace dll {

void call_fn_any(void* fn_ptr, Seq<Any> args) {
    auto vm = dll_internal::reset_vm();
    for (const auto& arg : args)
    {
        ASSERT(arg.v_type); // has to have a type
        ASSERT(arg.v_ptr); // has to have a value
        if (arg.v_type->is_primitive()) {
            auto uid = arg.v_type->uid;

            if (uid == CB_Bool::type->uid) {
                ASSERT(sizeof(CB_Bool::c_typedef) == sizeof(bool));
                dcArgBool(vm, *(bool*)arg.v_ptr); // 8 bit int

            } else if (uid == CB_i8::type->uid || uid == CB_u8::type->uid || uid == CB_Flag::type->uid) {
                ASSERT(sizeof(CB_i8::c_typedef) == sizeof(char));
                ASSERT(sizeof(CB_u8::c_typedef) == sizeof(char));
                ASSERT(sizeof(CB_Flag::c_typedef) == sizeof(char));
                dcArgChar(vm, *(char*)arg.v_ptr); // 8 bit int

            } else if (uid == CB_i16::type->uid || uid == CB_u16::type->uid) {
                ASSERT(sizeof(CB_i16::c_typedef) == sizeof(short));
                ASSERT(sizeof(CB_u16::c_typedef) == sizeof(short));
                dcArgShort(vm, *(short*)arg.v_ptr); // 16 bit int

            } else if (uid == CB_i32::type->uid || uid == CB_u32::type->uid) {
                ASSERT(sizeof(CB_i32::c_typedef) == sizeof(long));
                ASSERT(sizeof(CB_u32::c_typedef) == sizeof(long));
                dcArgLong(vm, *(long*)arg.v_ptr); // 32 bit int

            } else if (uid == CB_i64::type->uid || uid == CB_Int::type->uid || uid == CB_u64::type->uid || uid == CB_Uint::type->uid) {
                ASSERT(sizeof(CB_i64::c_typedef) == sizeof(long long));
                ASSERT(sizeof(CB_Int::c_typedef) == sizeof(long long));
                ASSERT(sizeof(CB_u64::c_typedef) == sizeof(long long));
                ASSERT(sizeof(CB_Uint::c_typedef) == sizeof(long long));
                dcArgLongLong(vm, *(long long*)arg.v_ptr); // 64 bit int

            } else if (uid == CB_f32::type->uid) {
                ASSERT(sizeof(CB_f32::c_typedef) == sizeof(float));
                dcArgFloat(vm, *(float*)arg.v_ptr); // 32 bit float

            } else if (uid == CB_f64::type->uid || uid == CB_Float::type->uid) {
                ASSERT(sizeof(CB_f64::c_typedef) == sizeof(double));
                ASSERT(sizeof(CB_Float::c_typedef) == sizeof(double));
                dcArgDouble(vm, *(double*)arg.v_ptr); // 64 bit double

            } else {
                // assume it's a pointer that should be passed by value
                // CB_String, CB_Pointer or CB_Function
                ASSERT(arg.v_type->cb_sizeof() == sizeof(void*));
                dcArgPointer(vm, *(void**)arg.v_ptr);
            }

            // dcArgInt(vm, *(int*)arg.v_ptr); // int (16-64 bit int) // not used (platform specific size)

        } else {
            // push pointer (const reference)
            dcArgPointer(vm, (void*)arg.v_ptr); // cast to remove const
        }
    }
}


} // namespace dll

