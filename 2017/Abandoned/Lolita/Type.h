#pragma once
#include "ObjectUtil.h"
#include "AstObject.h"
#include "AstModel.h"
#include "CompilerConfig.h"
#include <optional>
#include <variant>
#include <map>
#include <vector>
#include <cassert>
#include <memory>
#include <unordered_map>

namespace lolita
{
	// Forward Decl
	//
	class CType;
	class PrimaryType;
	class PointerType;
	class ArrayType;
	class FunctionType;
	// class AggregateType;
	// class EnumType;

	struct QualType
	{
		CType *Type;
		TypeQualifier Quals;
	};
	using QualTypeVec = std::vector<QualType>;

	// Hash Utilities
	//

	using TypeHash = uint32_t;

	TypeHash RandomTypeHash();
	TypeHash HashPointerType(QualType base);
	TypeHash HashArrayType(QualType base, size_t cnt);
	TypeHash HashFunctionType(QualType ret, const QualTypeVec& params, bool va_args);

	size_t CalcBuiltinTypeSize(BuiltinType type);

	// TypeTable
	//

	class TypeTable : NonMovable
	{
	public:
		TypeTable();

		CType* MakeBuiltin(BuiltinType type);

		CType* MakePointer(QualType type);
		CType* MakeArray(QualType type, size_t cnt);
		CType* MakeFunction(QualType ret_type, const QualTypeVec& params, bool va_args = false);

	private:
		void PushBuiltin(BuiltinType type);

		template <typename T>
		using TypeMultimap = std::unordered_multimap<TypeHash, std::unique_ptr<T>>;

		std::vector<PrimaryType> builtins_;
		TypeMultimap<PointerType> ptr_arena_;
		TypeMultimap<ArrayType> arr_arena_;
		TypeMultimap<FunctionType> func_arena_;
	};

	// CType Base Class
	//

	class CType
	{
	public:
		CType(TypeHash hash)
			: hash_(hash) { }

		virtual ~CType() = default;

		// if the type is builtin
		// for most of types are not primary
		// default implementation returns false
		virtual bool IsBuiltin() const { return false; }

		virtual bool IsVoid() const { return false; }
		virtual bool IsInteger() const { return false; }
		virtual bool IsFloat() const { return false; }
		virtual bool IsPointer() const { return false; }
		virtual bool IsScalar() const { return false; }
		virtual bool IsArray() const { return false; }
		virtual bool IsFunction() const { return false; }

		// if all meta-info known of the type
		// only complete type can yield a size
		bool IsComplete() const { return size_ != 0; }

		// FIXME: perhaps hash should be calculated every time called instead of cached?
		TypeHash Hash() const { return hash_; }
		const std::string& Name() const { return name_; }

		// calculate size of the type
		// returns 0 if type is incomplete
		size_t Size() const { return size_; }

		// default alignment of the type
		// returns 0 if type is incomplete
		size_t Alignment() const { return align_; }

		QualType MakeQualified(TypeQualifier quals = {})
		{
			return QualType{ this, quals };
		}

		virtual CType* ShiftBase(CType* old_base, CType* new_base) const;

	protected:
		void SetName(const std::string &name)
		{
			name_ = name;
		}

		void SetSize(size_t sz)
		{
			assert(sz > 0 && size_ == 0);

			size_ = sz;
			for (align_ = 1; align_ < size_; align_ <<= 1) { }
		}

		void SetAlign(size_t align)
		{
			assert(size_ > 0);

			align_ = align;
		}

	private:
		const TypeHash hash_;
		std::string name_ = {};

		size_t size_ = 0;
		size_t align_ = 0;
	};

	class PrimaryType : public CType
	{
	public:
		PrimaryType(BuiltinType type)
			: CType(RandomTypeHash()), type_(type)
		{

		}

		bool IsBuiltin() const override { return true; }
		bool IsInteger() const override
		{
			switch (type_)
			{
			case BuiltinType::Void:
			case BuiltinType::Float32:
			case BuiltinType::Float64:
				return false;
			default:
				return true;
			}
		}
		bool IsFloat() const override
		{
			return type_ == BuiltinType::Float32
				|| type_ == BuiltinType::Float64;
		}

	private:
		BuiltinType type_;
	};

	class PointerType : public CType
	{
	public:
		PointerType(QualType base)
			: CType(HashPointerType(base))
			, base_type_(base) 
		{
			assert(base.Type != nullptr);
			SetSize(kPointerSize);
		}

		auto BaseType() const { return base_type_; }

		bool IsPointer() const override { return true; }

		virtual CType* ShiftBase(CType* old_base, CType* new_base) const override;

	private:
		QualType base_type_;
	};

	class ArrayType : public CType
	{
	public:
		ArrayType(QualType base, size_t count)
			: CType(HashArrayType(base, count))
			, base_type_(base)
			, count_(count)
		{
			assert(base.Type != nullptr && base.Type->IsComplete());

			if (count > 0)
			{
				SetSize(base.Type->Size() * count_);
			}
		}

		auto BaseType() const { return base_type_; }
		auto Count() const { return count_; }

		virtual CType* ShiftBase(CType* old_base, CType* new_base) const override;

	private:
		QualType base_type_;
		size_t count_;
	};

	class FunctionType : public CType
	{
	public:
		FunctionType(QualType ret, 
			std::vector<QualType> params,
			bool va_args)
			: CType(HashFunctionType(ret, params, va_args))
			, ret_type_(ret)
			, param_types_(params)
			, va_args_(va_args)
		{
			// FIXME:
		}

		auto ReturnType() const { return ret_type_; }
		const auto& ParamTypes() const { return param_types_; }
		auto VaArgs() const { return va_args_; }

		virtual CType* ShiftBase(CType* old_base, CType* new_base) const override;
	private:
		QualType ret_type_;
		std::vector<QualType> param_types_;
		bool va_args_;
	};

	/*

	enum class AggregationLayout
	{
		// for struct
		Struct,
		// for union
		Union,
	};

	class AggregationTypeList
	{
	private:
		struct Item
		{
			std::string Name;
			QualifiedType Type;
			size_t Alignment;
			std::optional<size_t> BitFieldWidth;
		};

		std::vector<Item> members_;

	public:

	};

	class AggregateType : public CType
	{
	private:
		AggregationLayout layout_;

	public:
		auto Layout() const { return layout_; }

		void Define(AggregationTypeList list);
	};

	class EnumType : public CType
	{

	};
	//*/
}