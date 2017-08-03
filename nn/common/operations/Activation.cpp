/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Operations.h"
#include "OperationsUtils.h"

#include "internal/optimized/optimized_ops.h"

namespace android {
namespace nn {

bool genericActivationPrepare(const Shape& input,
                              Shape* output) {
    DCHECK_EQ(getNumberOfDimensions(input), 4);
    return SetShape(input, output);
}

bool reluFloat32(const float* inputData, const Shape& inputShape,
                 float* outputData, const Shape& outputShape) {
    int numElements = getNumberOfElements(inputShape);
    for (int i=0; i<numElements; i++, inputData++, outputData++) {
        *outputData = std::max(0.f, *inputData);
    }
    return true;
}

bool relu6Float32(const float* inputData, const Shape& inputShape,
                  float* outputData, const Shape& outputShape) {
    int numElements = getNumberOfElements(inputShape);
    for (int i=0; i<numElements; i++, inputData++, outputData++) {
        *outputData = std::min(std::max(0.f, *inputData), 6.f);
    }
    return true;
}

bool tanhFloat32(const float* inputData, const Shape& inputShape,
                 float* outputData, const Shape& outputShape) {
    int numElements = getNumberOfElements(inputShape);
    for (int i=0; i<numElements; i++, inputData++, outputData++) {
        *outputData = std::tanh(*inputData);
    }
    return true;
}

bool logisticFloat32(const float* inputData, const Shape& inputShape,
                     float* outputData, const Shape& outputShape) {
    int numElements = getNumberOfElements(inputShape);
    for (int i=0; i<numElements; i++, inputData++, outputData++) {
        *outputData = 1.f / (1.f + std::exp(-*inputData));
    }
    return true;
}

bool logisticQuant8(const uint8_t* inputData, const Shape& inputShape,
                    uint8_t* outputData, const Shape& outputShape) {
    int numElements = getNumberOfElements(inputShape);
    static constexpr int kInputIntegerBits = 4;

    const double input_real_multiplier =
            inputShape.scale *
            static_cast<double>(1 << (31 - kInputIntegerBits));

    int32_t input_multiplier = 0;
    int32_t input_left_shift = 0;
    QuantizeMultiplierGreaterThanOne(input_real_multiplier,
                                     &input_multiplier,
                                     &input_left_shift);
    int32_t input_range_radius =
            CalculateInputRadius(kInputIntegerBits, input_left_shift);

    optimized_ops::Logistic(
            inputData, convertShapeToDims(inputShape),
            inputShape.offset, input_range_radius,
            input_multiplier, input_left_shift,
            outputData, convertShapeToDims(outputShape));

    return true;
}

}  // namespace nn
}  // namespace android
