#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

/**
 * @brief Perform binary erosion with disk structuring element
 *
 * @ingroup morphology segmentation
 */
class FAST_EXPORT Erosion : public ProcessObject {
    FAST_PROCESS_OBJECT(Erosion)
    public:
        FAST_CONSTRUCTOR(Erosion, int, size, = 3)
        /**
         * Set size of structuring element, must be odd
         * @param size
         */
        void setStructuringElementSize(int size);
        void loadAttributes() override;
    private:
        void execute() override;

        int mSize;

    };
}
