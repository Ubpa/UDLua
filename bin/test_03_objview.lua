-- Manual registration
local C = UDRefl.RegisterType({
    type = "C",
    methods = {
        {
            name = "PrintType",
            flag = "Static",
            params = {"Ubpa::UDRefl::ObjectView"},
            body = function (obj)
                print(obj:GetType():GetName())
            end
        }
    }
})

ObjectView.new(C).PrintType(1) -- prints "int64"
ObjectView.new(C).PrintType(5.0) -- prints "float64"

-- clear
collectgarbage()
ObjectView.ReflMngr.typeinfos:erase(C)
