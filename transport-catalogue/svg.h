#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace svg {
    struct Rgb {
        Rgb() = default;
        Rgb(uint8_t r, uint8_t g, uint8_t b)
            : red(r), green(g), blue(b) { }
        Rgb(int r, int g, int b)
            : red(r), green(g), blue(b) { }
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba {
        Rgba() = default;
        Rgba(uint8_t r, uint8_t g, uint8_t b, double o)
            : red(r), green(g), blue(b), opacity(o) {}
        Rgba(int r, int g, int b, double o)
            : red(r), green(g), blue(b), opacity(o) {}
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

    inline const Color NoneColor = { };

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<< (std::ostream& out, StrokeLineCap elem);
    std::ostream& operator<< (std::ostream& out, StrokeLineJoin elem);
    std::ostream& operator<< (std::ostream& out, Color elem);

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    struct RenderContext {
        RenderContext(std::ostream& out) : out(out) { }
        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out), indent_step(indent_step), indent(indent) { }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    class Object {
    public:             // constructors
        void Render(const RenderContext& context) const;
        virtual ~Object() = default;
    private:            // methods
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    template <typename Owner>
    class PathProps {
    private:                // fields
        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_linecap_;
        std::optional<StrokeLineJoin> stroke_linejoin_;

    public:                 // methods
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {
            stroke_width_ = width;
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_linecap_ = line_cap;
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_linejoin_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv << *fill_color_ << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv << *stroke_color_ << "\""sv;
            }
            if (stroke_width_) {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if (stroke_linecap_) {
                out << " stroke-linecap=\""sv << *stroke_linecap_ << "\""sv;
            }
            if (stroke_linejoin_) {
                out << " stroke-linejoin=\""sv << *stroke_linejoin_ << "\""sv;
            }
        }

    private:            // methods
        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }
    };

    class Circle final : public Object, public PathProps<Circle> {
        \
    private:            // fields
        Point center_;
        double radius_ = 1.0;

    public:             // methods
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:            // methods
        void RenderObject(const RenderContext& context) const override; \
    };

    class Polyline final : public Object, public PathProps<Polyline> {
    private:            // fields
        std::vector<Point> points_;

    public:             // methods
        Polyline& AddPoint(Point point);

    private:            // methods
        void RenderObject(const RenderContext& context) const override;
    };

    class Text final : public Object, public PathProps<Text> {
    private:            // fields
        Point position_;
        Point offset_;
        uint32_t font_size_ = 1;
        std::string font_weight_;
        std::string font_family_;
        std::string data_;

    public:             // methods
        Text& SetPosition(Point pos);
        Text& SetOffset(Point offset);
        Text& SetFontSize(uint32_t size);
        Text& SetFontFamily(std::string font_family);
        Text& SetFontWeight(std::string font_weight);
        Text& SetData(std::string data);

    private:            // methods
        void RenderObject(const RenderContext& context) const override;
    };

    class ObjectContainer {
    public:
        template <typename Obj>
        void Add(Obj object) { AddPtr(std::make_unique<Obj>(object)); }
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
        virtual ~ObjectContainer() = default;
    };

    class Document : public ObjectContainer {
    private:            // fields
        std::vector<std::unique_ptr<Object>> objects_;

    public:             // methods
        void AddPtr(std::unique_ptr<Object>&& obj) override;
        void Render(std::ostream& out) const;
    };

    class Drawable {
    public:
        Drawable() = default;
        virtual void Draw(ObjectContainer& object) const = 0;
        virtual ~Drawable() = default;
    };
}  // namespace svg