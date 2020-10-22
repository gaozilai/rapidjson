#include "archiver.h"
#include <iostream>
#include <vector>

//////////////////////////////////////////////////////////////////////////////
// Test1: simple object

struct Student {
    Student() : name(), age(), height(), canSwim() {}
    Student(const std::string name, unsigned age, double height, bool canSwim) : name(name), age(age), height(height), canSwim(canSwim) {}

    void Write(JsonWriter& writer)
    {
        writer.StartObject();
        writer.Member("name") = name;
        writer.Member("age") = age;
        writer.Member("height") = height;
        writer.Member("canSwim") = canSwim;
        writer.EndObject();
    }

    void Read(JsonReader& reader)
    {
        reader.StartObject();
        reader.FindMember("name") >> name;
        reader.FindMember("age") >> age;
        reader.FindMember("height") >> height;
        reader.FindMember("canSwim") >> canSwim;
        reader.EndObject();
    }

    std::string name;
    unsigned age;
    double height;
    bool canSwim;
};

std::ostream& operator<<(std::ostream& os, const Student& s) {
    return os << s.name << " " << s.age << " " << s.height << " " << s.canSwim;
}

void test1() {
    std::string json;

    // Serialize
    {
        Student s("Lua", 9, 150.5, true);
        JsonWriter writer;
        s.Write(writer);
        json = writer.GetString();
        std::cout << json << std::endl;
    }

    // Deserialize
    {
        Student s;
        JsonReader reader(json.c_str());
        s.Read(reader);
        std::cout << s << std::endl;
    }
}

//////////////////////////////////////////////////////////////////////////////
// Test2: std::vector <=> JSON array
// 
// You can map a JSON array to other data structures as well

struct Group {
    Group() : groupName(), students() {}
    std::string groupName;
    std::vector<Student> students;

    void Write(JsonWriter& writer)
    {
        size_t studentCount = students.size();

        writer.StartObject();
        writer.Member("groupName") = groupName;
        writer.Member("students").StartArray(&studentCount);
        for (size_t i = 0; i < studentCount; i++)
            students[i].Write(writer);
        writer.EndArray();
        writer.EndObject();
    }

    void Read(JsonReader& reader)
    {
        reader.StartObject();
        reader.FindMember("groupName") >> groupName;
        reader.FindMember("students");
        size_t studentCount = 0;
        reader.StartArray(&studentCount);
        students.resize(studentCount);
        for (size_t i = 0; i < studentCount; i++)
            students[i].Read(reader);
        reader.EndArray();
        reader.EndObject();
    }
};

std::ostream& operator<<(std::ostream& os, const Group& g) {
    os << g.groupName << std::endl;
    for (std::vector<Student>::const_iterator itr = g.students.begin(); itr != g.students.end(); ++itr)
        os << *itr << std::endl;
    return os;
}

void test2() {
    std::string json;

    // Serialize
    {
        Group g;
        g.groupName = "Rainbow";

        Student s1("Lua", 9, 150.5, true);
        Student s2("Mio", 7, 120.0, false);
        g.students.push_back(s1);
        g.students.push_back(s2);

        JsonWriter writer;
        g.Write(writer);

        json = writer.GetString();
        std::cout << json << std::endl;
    }

    // Deserialize
    {
        Group g;
        JsonReader reader(json.c_str());
        g.Read(reader);
        std::cout << g << std::endl;
    }
}

//////////////////////////////////////////////////////////////////////////////
// Test3: polymorphism & friend
//
// Note that friendship is not necessary but make things simpler.

class Shape {
public:
    virtual ~Shape() {}
    virtual const char* GetType() const = 0;
    virtual void Print(std::ostream& os) const = 0;
    virtual void Write(JsonWriter& writer) = 0;
    virtual void Read(JsonReader& reader) = 0;

protected:
    Shape() : x_(), y_() {}
    Shape(double x, double y) : x_(x), y_(y) {}

    double x_, y_;
};

class Circle : public Shape {
public:
    Circle() : radius_() {}
    Circle(double x, double y, double radius) : Shape(x, y), radius_(radius) {}
    ~Circle() {}

    const char* GetType() const { return "Circle"; }

    void Print(std::ostream& os) const {
        os << "Circle (" << x_ << ", " << y_ << ")" << " radius = " << radius_;
    }

    void Write(JsonWriter& writer)
    {
        std::string type = GetType();
        writer.StartObject();
        writer.Member("type") = type;
        writer.Member("x") = x_;
        writer.Member("y") = y_;
        writer.Member("radius") = radius_;
        writer.EndObject();
    }

    void Read(JsonReader& reader)
    {
        reader.FindMember("x") >> x_;
        reader.FindMember("y") >> y_;
        reader.FindMember("radius") >> radius_;
    }

private:
    double radius_;
};

class Box : public Shape {
public:
    Box() : width_(), height_() {}
    Box(double x, double y, double width, double height) : Shape(x, y), width_(width), height_(height) {}
    ~Box() {}

    const char* GetType() const { return "Box"; }

    void Print(std::ostream& os) const {
        os << "Box (" << x_ << ", " << y_ << ")" << " width = " << width_ << " height = " << height_;
    }

    void Write(JsonWriter& writer)
    {
        std::string type = GetType();
        writer.StartObject();
        writer.Member("type") = type;
        writer.Member("x") = x_;
        writer.Member("y") = y_;
        writer.Member("width") = width_;
        writer.Member("height") = height_;

        writer.EndObject();
    }
    void Read(JsonReader& reader)
    {
        reader.FindMember("x") >> x_;
        reader.FindMember("y") >> y_;
        reader.FindMember("width") >> width_;
        reader.FindMember("height") >> height_;
    }

private:
    double width_, height_;
};

class Canvas {
public:
    Canvas() : shapes_() {}
    ~Canvas() { Clear(); }
    
    void Clear() {
        for (std::vector<Shape*>::iterator itr = shapes_.begin(); itr != shapes_.end(); ++itr)
            delete *itr;
    }

    void AddShape(Shape* shape) { shapes_.push_back(shape); }
    
    void Print(std::ostream& os) {
        for (std::vector<Shape*>::iterator itr = shapes_.begin(); itr != shapes_.end(); ++itr) {
            (*itr)->Print(os);
            std::cout << std::endl;
        }
    }

    void Write(JsonWriter& writer)
    {
        size_t shapeCount = shapes_.size();

        writer.StartArray(&shapeCount);
        for (size_t i = 0; i < shapeCount; i++)
            shapes_[i]->Write(writer);
        writer.EndArray();
    }
    
    void Read(JsonReader& reader)
    {
        size_t shapeCount = 0;
        reader.StartArray(&shapeCount);
        Clear();
        shapes_.resize(shapeCount);

        for (size_t i = 0; i < shapeCount; i++)
        {
            std::string type;
            reader.StartObject();
            reader.FindMember("type") >> type;
            if (type == "Circle") 
            {
                shapes_[i] = new Circle;
            }
            else if (type == "Box")
            {
                shapes_[i] = new Box;
            }
            shapes_[i]->Read(reader);

            reader.EndObject();
        }
        reader.EndArray();
    }

private:
    std::vector<Shape*> shapes_;
};

void test3() {
    std::string json;

    // Serialize
    {
        Canvas c;
        c.AddShape(new Circle(1.0, 2.0, 3.0));
        c.AddShape(new Box(4.0, 5.0, 6.0, 7.0));

        JsonWriter writer;
        c.Write(writer);
        json = writer.GetString();
        std::cout << json << std::endl;
    }

    // Deserialize
    {
        Canvas c;
        JsonReader reader(json.c_str());
        c.Read(reader);
        c.Print(std::cout);
    }
}

int main() {
    test1();
    test2();
    test3();
}
