--ref https://github.com/openresty/lua-resty-upload

local upload = require "resty.upload"
--local uuid = require("resty.uuid")
assert(upload ~= nil)

local dst_dir = "./"

local function my_get_file_name(header)
    local file_name
    for i, ele in ipairs(header) do
        file_name = string.match(ele, 'filename="(.*)"')
        if file_name and file_name ~= '' then
            return file_name
        end
    end
    return nil
end

local chunk_size = 4096

local form = upload:new(chunk_size)
if not form then
    ngx.log(ngx.ERR, "failed to new upload: ", err)

    return 
end

local file
local file_path
while true do
    local typ, res, err = form:read()

    if not typ then
         ngx.say("failed to read: ", err)
         return
    end

    if typ == "header" then
        local file_name = my_get_file_name(res)
        if file_name then
            --file_name = uuid.generate()
            file_path = dst_dir..file_name
            print('file_path: ', file_path)
            file = io.open(file_path, "w+")
            if not file then
                ngx.say("failed to open file ", file_path)
                return
            end
        else
            ngx.log(ngx.ERR, "file_name is nil")
            return
        end

    elseif typ == "body" then
        if file then
            file:write(res)
        end

    elseif typ == "part_end" then
        if file then
            file:close()
            file = nil
            ngx.say("upload to "..file_path.." successfully!")
        end
    elseif typ == "eof" then
        break

    else
        -- do nothing
    end
end