#include "Type.h"
#include <random>

namespace lolita
{
	// Helpers
	//

	extern TypeTable& GetTypeTable();

	bool operator==(TypeQualifier lhs, TypeQualifier rhs)
	{
		return lhs.Const == rhs.Const
			&& lhs.Restrict == rhs.Restrict
			&& lhs.Volatile == rhs.Volatile;
	}

	bool operator==(QualType lhs, QualType rhs)
	{
		return lhs.Type == rhs.Type
			&& lhs.Quals.Const == rhs.Quals.Const
			&& lhs.Quals.Restrict == rhs.Quals.Restrict
			&& lhs.Quals.Volatile == rhs.Quals.Volatile;
	}

	// Hash Functions
	//
	TypeHash HushQualType(QualType type)
	{
		auto result = type.Type->Hash();

		if (type.Quals.Const)
			result *= 17;
		if (type.Quals.Restrict)
			result *= 23;
		if (type.Quals.Volatile)
			result *= 31;

		return result;
	}

	TypeHash RandomTypeHash()
	{
		// FIXME: thread_local?
		static auto rnd_engine = std::minstd_rand{};
		return rnd_engine();
	}
	TypeHash HashPointerType(QualType base)
	{
		return 0x8f9d7e63 ^ HushQualType(base) + 0x1234;
	}
	TypeHash HashArrayType(QualType base, size_t cnt)
	{
		return (0x6c7d2333 + 37 * cnt) ^ HushQualType(base) + 0xfe41;
	}
	TypeHash HashFunctionType(QualType ret, const QualTypeVec& params, bool va_args) 
	{
		auto result = HushQualType(ret);
		for (auto type : params)
		{
			result *= 11;
			result ^= HushQualType(type);
		}
		
		if (va_args)
			result *= 19;

		return result;
	}

	// Other Functions
	//

	size_t CalcBuiltinTypeSize(BuiltinType type)
	{
		switch (type)
		{
		case BuiltinType::Boolean:
		case BuiltinType::Char:
		case BuiltinType::Int8:
		case BuiltinType::UInt8:
			return 1;
		case BuiltinType::Int16:
		case BuiltinType::UInt16:
			return 2;
		case BuiltinType::Int32:
		case BuiltinType::UInt32:
		case BuiltinType::Float32:
			return 4;
		case BuiltinType::Int64:
		case BuiltinType::UInt64:
		case BuiltinType::Float64:
			return 8;
		}
	}

	auto CreateBuiltinTypeVec()
	{
		std::vector<PrimaryType> result;

		// init builtins
		// NOTE order of initialization is important
		result.emplace_back(BuiltinType::Void);

		result.emplace_back(BuiltinType::Boolean);
		result.emplace_back(BuiltinType::Char);

		result.emplace_back(BuiltinType::Int8);
		result.emplace_back(BuiltinType::Int16);
		result.emplace_back(BuiltinType::Int32);
		result.emplace_back(BuiltinType::Int64);

		result.emplace_back(BuiltinType::UInt8);
		result.emplace_back(BuiltinType::UInt16);
		result.emplace_back(BuiltinType::UInt32);
		result.emplace_back(BuiltinType::UInt64);

		result.emplace_back(BuiltinType::Float32);
		result.emplace_back(BuiltinType::Float64);

		return result;
	}

	// TypeTable Impl
	//

	TypeTable::TypeTable()
		: builtins_(CreateBuiltinTypeVec()) { }

	CType* TypeTable::MakeBuiltin(BuiltinType type)
	{
		auto index = static_cast<size_t>(type);

		assert(index < builtins_.size());
		return &builtins_[index];
	}

	// FIXME: TypeTable::Make* functions are very similar
	CType* TypeTable::MakePointer(QualType base)
	{
		// calculate hash
		auto hash = HashPointerType(base);
		
		// find ptr type in cache
		auto iter_pair = ptr_arena_.equal_range(hash);
		for (auto iter = iter_pair.first; iter != iter_pair.second; ++iter)
		{
			auto type = iter->second.get();
			if (type->BaseType() == base)
			{
				return type;
			}
		}

		// then type is not yet created
		// so construct one
		auto result_iter = ptr_arena_.insert(
			std::make_pair(hash, std::make_unique<PointerType>(base))
		);

		return result_iter->second.get();
	}
	
	CType* TypeTable::MakeArray(QualType base, size_t cnt)
	{
		// calculate hash
		auto hash = HashArrayType(base, cnt);

		// find array type in cache
		auto iter_pair = arr_arena_.equal_range(hash);
		for (auto iter = iter_pair.first; iter != iter_pair.second; ++iter)
		{
			auto type = iter->second.get();
			if (type->BaseType() == base
				&& type->Count() == cnt)
			{
				return type;
			}
		}

		// then type is not yet created
		// so construct one
		auto result_iter = arr_arena_.insert(
			std::make_pair(hash, std::make_unique<ArrayType>(base, cnt))
		);

		return result_iter->second.get();
	}
	
	CType* TypeTable::MakeFunction(QualType ret, const QualTypeVec& params, bool va_args)
	{
		// calculate hash
		auto hash = HashFunctionType(ret, params, va_args);

		// find function type in cache
		auto iter_pair = func_arena_.equal_range(hash);
		for (auto iter = iter_pair.first; iter != iter_pair.second; ++iter)
		{
			auto type = iter->second.get();
			if (type->ReturnType() == ret
				&& type->ParamTypes() == params
				&& type->VaArgs() == va_args)
			{
				return type;
			}
		}

		// then type is not yet created
		// so construct one
		auto result_iter = func_arena_.insert(
			std::make_pair(hash, std::make_unique<FunctionType>(ret, params, va_args))
		);

		return result_iter->second.get();
	}

	// ShiftBase Impl
	//

	CType* CType::ShiftBase(CType* old_base, CType* new_base) const
	{
		// NOTE instance of CType is unique globally
		// managed by a TypeTable instance managed by TranslationContext
		if (this == old_base)
			return new_base;
		else
			throw 0;
	}

	CType* PointerType::ShiftBase(CType* old_base, CType* new_base) const
	{
		if (this == old_base)
			return new_base;

		return GetTypeTable().MakePointer(
			base_type_.Type
			->ShiftBase(old_base, new_base)
			->MakeQualified(base_type_.Quals)
		);
	}

	CType* ArrayType::ShiftBase(CType* old_base, CType* new_base) const
	{
		if (this == old_base)
			return new_base;

		return GetTypeTable().MakeArray(
			base_type_.Type
			->ShiftBase(old_base, new_base)
			->MakeQualified(base_type_.Quals),

			count_
		);
	}

	CType* FunctionType::ShiftBase(CType* old_base, CType* new_base) const
	{
		if (this == old_base)
			return new_base;

		return GetTypeTable().MakeFunction(
			ret_type_.Type
			->ShiftBase(old_base, new_base)
			->MakeQualified(ret_type_.Quals),

			param_types_,
			va_args_
		);
	}
}