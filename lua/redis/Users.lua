function user_set_with_ttl()
    local user_id = KEYS[1]
    local json_data = ARGV[1]
    local ttl = tonumber(ARGV[2])

    redis.call("HSET", "user_info", user_id, json_data)
    redis.call("SETEX", "user_session_ttl:" .. user_id, ttl, "1") -- TTL 감시만

    return 1
end

-- 유저 정보 삭제
function user_delete(user_id)
    redis.call("HDEL", "user_info", user_id)
    redis.call("DEL", "user_ttl:" .. user_id)
    return 1
end

-- 유저 정보 조회
function user_get(user_id)
    return redis.call("HGET", "user_info", user_id)
end

-- 유저 존재 여부 확인
function user_exists(user_id)
    return redis.call("HEXISTS", "user_info", user_id)
end



-- Dispatcher
local command = ARGV[1]
local user_id = ARGV[2]

if command == "set" then
    local json_data = ARGV[3]
    local ttl = ARGV[4]
    return user_set_with_ttl(user_id, json_data, ttl)
elseif command == "del" then
    return user_delete(user_id)
elseif command == "get" then
    return user_get(user_id)
elseif command == "exists" then
    return user_exists(user_id)
else
    return redis.error_reply("Invalid command")
end