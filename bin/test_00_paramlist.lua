local vec = UDRefl.RegisterType({
    type = "vec",
    fields = {
        { type = "float32", name = "x" },
        { type = "float32", name = "y" },
    },
    methods = {
        {
            name = "offset",
            result = "&{vec}",
            params = { "float32" },
            body = function (v, offset)
                v.x = v.x + offset
                v.y = v.y + offset
                return v
            end
        }
    }
})

local v = SharedObject.new(vec)
v.x = 1
v.y = 2
v:offset(3)
print(v.x .. ", " .. v.y)

-- clear
v = nil
collectgarbage()
ObjectView.ReflMngr.typeinfos:erase(vec)
