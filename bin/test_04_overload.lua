-- Manual registration
local vec = UDRefl.RegisterType({
    type = "vec",
    fields = {
        { type = "float32", name = "x" },
        { type = "float32", name = "y" },
    },
    methods = {
        {
            name = "__add",
            result = "vec",
            params = { "&{const{vec}}" },
            body = function (v, rhs)
                print("vec& vec::operator+(&{const{vec}})")
                rst = SharedObject.new("vec")
                rst.x = v.x + rhs.x
                rst.y = v.y + rhs.y
                return rst
            end
        },
        {
            name = "__add",
            result = "vec",
            params = { "float32" },
            body = function (v, rhs)
                print("vec& vec::operator+(float32)")
                rst = SharedObject.new("vec")
                rst.x = v.x + rhs
                rst.y = v.y + rhs
                return rst
            end
        }
    }
})

for iter in ObjectView.new(vec):GetFields():__range() do
    local name, info = iter:__indirection():__tuple_bind()
    print(name:GetView())
end

for iter in ObjectView.new(vec):GetMethods():__range() do
    local name, info = iter:__indirection():__tuple_bind()
    print(name:GetView())
end

local v = SharedObject.new(vec)

v.x = 3
v.y = 4

w0 = v + 1
print(w0.x, w0.y) -- 4, 5

w1 = v + w0
print(w1.x, w1.y) -- 7, 9

-- clear
v = nil
w0 = nil
w1 = nil
collectgarbage()
ObjectView.ReflMngr.typeinfos:erase(vec)
