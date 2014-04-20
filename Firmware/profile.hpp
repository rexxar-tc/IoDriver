#include <string>
#include <vector>

#include <color.hpp>

namespace Sabre {

class Profile {
public:
    Profile();
    Profile( const Profile& );
    ~Profile();

    uint8_t get_index();
    std::string get_name();
    const std::vector<Sabre::ColorData>& get_colors();

    int init( signed int );

    rgbw get_gradient_value();
    void reset_gradient_value();

    void set_gradient( const std::vector<Sabre::ColorData>::iterator& );
    void next_gradient();
    void first_gradient();

protected:

private:
    std::string pname;
    std::vector<Sabre::ColorData> colors;
    uint8_t index;

    std::vector<Sabre::ColorData>::iterator activeGradient;
};

}
