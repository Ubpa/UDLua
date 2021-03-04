local vec = UDRefl.RegisterType({
    type = "vec",
    fields = {
        {
            type = "float32",
            name = "x"
        },
        {
            type = "float32",
            name = "y"
        },
    }
})
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
