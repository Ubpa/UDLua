local Info = UDRefl.RegisterType({
    type = "Info",
    fields = {
        { type = "std::string", name = "value" }
    },
    methods = {
        {
            name = "__ctor",
            params = { "std::string" },
            body = function (this, value)
                this.value:__ctor(value)
            end
        }
    }
})

local C = UDRefl.RegisterType({
    type = "C",
    attrs = {
        {
            type = "Info",
            init_args = { "hello world" }
        }
    },
    fields = {
        {
            type = "float32",
            name = "data",
            attrs = {
                {
                    type = "Info",
                    init_args = { "I am a field attr" }
                }
            }
        }
    },
    unowned_fields = {
        {
            type = "float32",
            name = "unowned_data",
            attrs = {
                {
                    type = "Info",
                    init_args = { "I am a unowned field attr" }
                }
            }
        }
    },
    methods = {
        {
            name = "func",
            body = function () end,
            attrs = {
                {
                    type = "Info",
                    init_args = { "I am a method attr" }
                }
            }
        }
    }
})

print(ObjectView.ReflMngr:GetTypeAttr(C, Info).value) -- hello world
print(ObjectView.ReflMngr:GetFieldAttr(C, "data", Info).value) -- I am a field attr
print(ObjectView.ReflMngr:GetFieldAttr(C, "unowned_data", Info).value) -- I am a unowned field attr
print(ObjectView.ReflMngr:GetMethodAttr(C, "func", Info).value) -- I am a field attr

-- clear
collectgarbage()
ObjectView.ReflMngr.typeinfos:erase(C)
ObjectView.ReflMngr.typeinfos:erase(Info)
