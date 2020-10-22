// JSON simple example
// This example does not handle errors.

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <iostream>

using namespace rapidjson;

int main() {
    const char* json = R"({
	"Age" : 36,
	"E-mail" : 
	{
		"Hotmail" : "gao_zilao@hotmail.com",
		"Netease" : "utrust@163.com"
	},
	"Language" : 
	[
		"C",
		"C++",
		"Java",
		"Python",
		{
			"Visual Studio" : 
			[
				"Visual C++",
				"Visual C#",
				"Visual F#"
			]
		},
		[
			"Visual C++",
			"Visual C#",
			"Visual F#"
		]
	],
	"Name" : "Gao zilai"
})";

    // 1. Parse a JSON string into DOM.
    //const char* json = "{\"project\":\"rapidjson\",\"stars\":10}";
    Document d;
    d.Parse(json);

    // 2. Modify it by DOM.
    Value& s = d["Age"];
    s.SetInt(s.GetInt() + 1);
    d["Language"][2] = "java";
    d["Language"][4]["Visual Studio"][0] = "C++";
    d["Language"][4]["Visual Studio"][1] = "C#";
    SizeType t = d["Language"][4]["Visual Studio"].Capacity();
    std::cout << "Number of array is" << t << std::endl;
    d["Language"][4]["Visual Studio"].Clear();

    d.RemoveMember("Name");


    // 3. Stringify the DOM
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);

    // Output {"project":"rapidjson","stars":11}
    std::cout << buffer.GetString() << std::endl;
    return 0;
}
