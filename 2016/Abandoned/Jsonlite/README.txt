Jsonlite is a lightweighted Json library written in C++, that enables you to easily construct or parse a Json-based document.
The utility is a part of my project CSimplified, to help generation of in-text intermediate output of a compiler component, .eg token stream, parse tree, and .etc.

[Parsing]
To parse a json entity, Jsonlite provides you with a fast and lazy API which is very easy to operate. Steps are following:
1. Load the document into memory in an std::istringstream object, say input.
2. eds::JsonParser parser(input);
3. auto value = parser.GetValue().Deserialize<type_you_want>();

The type your document should corespond to the specific type you provide. And if it's a custom type of your own, you have to specify the coresponding
template class of JsonSerializer as well. Given a struct Item:

struct Item {
	int Id;
	std::string Name;
	double Price;
	bool IsAvailable;
};

Instances of such struct corespond to an Object in Json. Say,
{
	"Id" : 0,
	"Name" : "Brush",
	"Price" : 7.99,
	"IsAvailable" : true
}

And your version of JsonSerializer may look like this:
template<>
struct JsonSerializer<Item> {
	void Serialize(JsonBuilder &builder, const Item &dat)
	{
		builder.FeedObject([&]() {
			
			builder.SerializeAsField("Id", dat.Id);
			builder.SerializeAsField("Name", dat.Name);
			builder.SerializeAsField("Price", dat.Price);
			builder.SerializeAsField("IsAvailable", dat.IsAvailable);
		});
	}

	Item Deserialize(const JsonValue &object)
	{
		Item tmp;
		object.AsObject([&tmp](const string &key, const JsonValue &value)
		{
			if (key == "Id")
			{
				tmp.Id = value.Deserialize<int>();
			}
			else if (key == "Name")
			{
				tmp.Name = value.Deserialize<string>();
			}
			else if (key == "Price")
			{
				tmp.Price = value.Deserialize<double>();
			}
			else if (key == "IsAvailable")
			{
				tmp.IsAvailable = value.Deserialize<bool>();
			}
		});

		return move(tmp);
	}
};

Pretty annoying, isn't it? But life is damn short and never that easy in C++, you know.
JsonSerializer<T> is an adapter bewteen Json and C++ native structure of T, which is
in charge of converting a JsonValue into T or writing an instance of T into JsonBuilder as well.
( As it's based on template specialization of C++, please define that in the global or eds namespace)
Inside the member function JsonSerializer<Item>::Deserialize(const JsonValue &object),
you can find a more basic interface that Jsonlite provide with you - A callback model to parse a Json object/array.
Taste if yourself.