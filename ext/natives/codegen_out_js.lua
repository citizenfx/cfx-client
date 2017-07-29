local function printFunctionName(native)
	return native.name:lower():gsub('0x', 'n_0x'):gsub('_(%a)', string.upper):gsub('(%a)(.+)', function(a, b)
		return a:upper() .. b
	end)
end

-- sort the natives table
local _natives = {}

for _, v in ipairs(natives) do
	table.insert(_natives, v)
end

table.sort(_natives, function(a, b)
	return printFunctionName(a) < printFunctionName(b)
end)

-- header bit
print("const _i = Citizen.pointerValueInt();")
print("const _f = Citizen.pointerValueFloat();")
print("const _v = Citizen.pointerValueVector();")
print("const _r = Citizen.returnResultAnyway();")
print("const _ri = Citizen.resultAsInteger();")
print("const _rf = Citizen.resultAsFloat();")
print("const _s = Citizen.resultAsString();")
print("const _rv = Citizen.resultAsVector();")
print("const _in = Citizen.invokeNative;")
print("const _ii = Citizen.pointerValueIntInitialized;")
print("const _fi = Citizen.pointerValueFloatInitialized;")

print("function _ts(num) {")
print("\tif (num == 0 || !num) { // workaround for users calling string parameters with '0', also nil being translated")
print("\t\treturn null;")
print("\t}")
print("\treturn num.toString();")
print("}")

print("function _ch(hash) {")
print("\tif (typeof hash === 'string') {")
print("\t\treturn window.GetHashKey(hash);")
print("\t}\n")
print("\treturn hash;")
print("}\n")

print("function _mfr(fn) {")
print("\treturn Citizen.makeRefFunction(fn);")
print("}\n")

print("const _ENV = null;\n")

-- functions
local function printArgumentName(name)
	if name == 'var' then
		return 'var_'
	end

	return name
end

local function trimAndNormalize(str)
	return trim(str):gsub('/%*', ' -- [['):gsub('%*/', ']] ')
end

local function isSinglePointerNative(native)
	local foundPointer = false

	for _, v in ipairs(native.arguments) do
		if v.pointer then
			if foundPointer then
				return false
			else
				foundPointer = true
			end
		end
	end

	return native.arguments[#native.arguments].pointer
end

local function printArgument(argument, native)
	if argument.pointer then
		if argument.type.nativeType == 'int' then
			if isSinglePointerNative(native) then
				return '_ii(' .. printArgumentName(argument.name) .. ') /* may be optional */'
			else
				return '_i'
			end
		elseif argument.type.nativeType == 'float' then
			if isSinglePointerNative(native) then
				return '_fi(' .. printArgumentName(argument.name) .. ') /* may be optional */'
			else
				return '_f'
			end
		elseif argument.type.nativeType == 'vector3' then
			return '_v'
		else
			return '_i /* actually ' .. argument.type.nativeType .. ' */'
		end
	elseif argument.type.name == 'func' then
		return '_mfr(' .. printArgumentName(argument.name) .. ')'
	elseif argument.type.name == 'Hash' then
		return '_ch(' .. printArgumentName(argument.name) .. ')'
	elseif argument.type.name == 'charPtr' then
		return '_ts(' .. printArgumentName(argument.name) .. ')'
	end

	return printArgumentName(argument.name)
end

local function printArgumentList(native)
	if not native.arguments then
		return ''
	end

	local args = {}

	for _, v in ipairs(native.arguments) do
		if not v.pointer or isSinglePointerNative(native) then
			table.insert(args, printArgumentName(v.name))
		end
	end

	return table.concat(args, ', ')
end

local function printReturnType(type)
	if type.nativeType == 'string' then
		return '_s'
	elseif type.nativeType == 'float' then
		return '_rf'
	elseif type.nativeType == 'vector3' then
		return '_rv'
	elseif type.nativeType == 'int' then
		return '_ri'
	elseif type.nativeType == 'Any*' then
		return '_ri'
	end
end

local function printInvocationArguments(native)
	local args = {
		'"' .. native.hash .. '"'
	}

	if native.arguments then
		for _, v in pairs(native.arguments) do
			table.insert(args, printArgument(v, native))
		end
	end

	if native.returns then
		table.insert(args, '_r')

		if native.returns.nativeType ~= 'bool' then
			table.insert(args, printReturnType(native.returns))
		end
	end

	return table.concat(args, ', ')
end

local function formatDocString(native)
	local d = parseDocString(native)

	if not d then
		return ''
	end

	local firstLine, nextLines = d.summary:match("([^\n]+)\n?(.*)")

	if not firstLine then
		return ''
	end

	local l = '/**\n * ' .. trimAndNormalize(firstLine) .. "\n"

	for line in nextLines:gmatch("([^\n]+)") do
		l = l .. ' * ' .. trimAndNormalize(line) .. "\n"
	end

	if d.hasParams then
		for n, v in pairs(d.params) do
			l = l .. ' * @param ' .. n .. ' ' .. v .. '\n'
		end
	end

	if d.returns then
		l = l .. ' * @return ' .. d.returns .. '\n'
	end

	l = l .. ' */\n'

	return l
end

local function printNative(native)
	local str = string.format("%swindow.%s = function (%s) {\n", formatDocString(native), printFunctionName(native), printArgumentList(native))

	str = str .. string.format("\treturn _in(%s);\n", printInvocationArguments(native))

	str = str .. "};\n"

	for _, alias in ipairs(native.aliases) do
		str = str .. ("window.%s = window.%s;\n"):format(printFunctionName({ name = alias }), printFunctionName(native))
	end

	return str
end

for _, v in pairs(_natives) do
	if matchApiSet(v) then
		print(printNative(v))
	end
end