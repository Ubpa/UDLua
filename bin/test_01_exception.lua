local vec = require("vec")
local paramlist = SharedObject.new(Type.new("std::vector<{Ubpa::Type},{std::allocator<{Ubpa::Type}>}>"))
paramlist:push_back(Type.new("float32"))
local minfo_vec_offset = SharedObject.new(Type.new("Ubpa::UDRefl::MethodInfo"))
minfo_vec_offset.methodptr = SharedObject.new_MethodPtr(
    vec,
    function (v, offset)
        v.x = v.x + offset
        v.y = v.y + offset
        return nil -- wrong return type
    end,
    Type.new("&{vec}"),
    paramlist
)
ObjectView.ReflMngr:AddMethod(vec, Name.new("offset"), minfo_vec_offset)

local v = SharedObject.new(vec)
v.x = 1
v.y = 2
v:offset(3) -- catch exception
print(v.x .. ", " .. v.y)

collectgarbage()
ObjectView.ReflMngr.typeinfos:erase(vec)
