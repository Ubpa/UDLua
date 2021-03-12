-- Manual registration
local vec = UDRefl.RegisterType({
    type = "vec",
    fields = {
        { type = "float32", name = "x" },
        { type = "float32", name = "y" },
    },
    methods = {
        {
            name = "norm",
            result = "float32",
            body = function (p)
                return math.sqrt(UDRefl.unbox(p.x * p.x + p.y * p.y))
            end
        }
    }
})

-- Iterate over members
for iter in ObjectView.new(vec):GetFields():__range() do
    local name, info = iter:__indirection():__tuple_bind()
    print(name:GetView())
end

for iter in ObjectView.new(vec):GetMethods():__range() do
    local name, info = iter:__indirection():__tuple_bind()
    print(name:GetView())
end

-- Constructing types
local v = SharedObject.new(vec)
print(v:GetType():GetName()) -- prints "vec"

-- Set/get variables
v.x = 3
v.y = 4
print("x: " .. v.x) -- prints "3"

-- Invoke Methods
print("norm: " .. v:norm()) -- prints "5"

-- Iterate over variables
for iter in v:GetVars():__range() do
    local name, var = iter:__indirection():__tuple_bind()
    print(name:GetView() .. ": ".. var)
end

-- clear
v = nil
collectgarbage()
ObjectView.ReflMngr.typeinfos:erase(vec)
