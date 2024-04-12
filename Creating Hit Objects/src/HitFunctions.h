#pragma once
#include <cstdint>

namespace Hit_org{
    class Hit{
        public:
            float x;
            float y;
            float z;
    };

    Hit_org::Hit create_hit_obj(std::int64_t i);
}