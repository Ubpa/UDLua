local vec = Type.new("vec")
ObjectView.ReflMngr:RegisterType(vec, 8, 4)
local finfo_x = SharedObject.new(Type.new("Ubpa::UDRefl::FieldInfo"))
finfo_x.fieldptr = SharedObject.new(Type.new("Ubpa::UDRefl::FieldPtr"), Type.new("float32"), 0)
local finfo_y = SharedObject.new(Type.new("Ubpa::UDRefl::FieldInfo"))
finfo_y.fieldptr = SharedObject.new(Type.new("Ubpa::UDRefl::FieldPtr"), Type.new("float32"), 4)
ObjectView.ReflMngr:AddField(vec, Name.new("x"), finfo_x)
ObjectView.ReflMngr:AddField(vec, Name.new("y"), finfo_y)
ObjectView.ReflMngr:AddTrivialDefaultConstructor(vec)
local minfo_vec_norm2 = SharedObject.new(Type.new("Ubpa::UDRefl::MethodInfo"))
minfo_vec_norm2.methodptr = SharedObject.new_MethodPtr(
    vec,
    function (p)
        return p.x * p.x + p.y * p.y
    end,
    Type.new("float32")
)
ObjectView.ReflMngr:AddMethod(vec, Name.new("norm2"), minfo_vec_norm2)
return vec
