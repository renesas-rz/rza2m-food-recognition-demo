/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
/* Copyright 2020 Renesas Electronics

Renesas has modified part of this file. The following comments have been added to
the modifies / additions.

	start 	Modified by Renesas Electronics {
	end   	Modified by Renesas Electronics }
==============================================================================*/
/* Modified by Renesas Electronics { */
#ifndef TENSORFLOW_LITE_MICRO_EXAMPLES_FOOD_MENU_RECOGNITION_MODEL_SETTINGS_H_
#define TENSORFLOW_LITE_MICRO_EXAMPLES_FOOD_MENU_RECOGNITION_MODEL_SETTINGS_H_

// Keeping these as constant expressions allow us to allocate fixed-sized arrays
// on the stack for our working memory.

// All of these values are derived from the values used during model training,
// if you change your model you'll need to update these constants.
constexpr int kNumCols = 128;
constexpr int kNumRows = 128;
constexpr int kNumChannels = 3;

constexpr int kMaxImageSize = kNumCols * kNumRows * kNumChannels;
constexpr int kCategoryCount = 15;

// Index=15

constexpr int k_caesar_salad        = 0;
constexpr int k_club_sandwich       = 1;
constexpr int k_donuts              = 2;
constexpr int k_edamame             = 3;
constexpr int k_french_fries        = 4;
constexpr int k_fried_rice          = 5;
constexpr int k_hamburger           = 6;
constexpr int k_hot_dog             = 7;
constexpr int k_oysters             = 8;
constexpr int k_pancakes            = 9;
constexpr int k_pizza               = 10;
constexpr int k_ramen               = 11;
constexpr int k_spaghetti_carbonara = 12;
constexpr int k_steak               = 13;
constexpr int k_sushi               = 14;

extern const char* kCategoryLabels[kCategoryCount];
extern const char* kCategoryPrices[kCategoryCount];

#endif  // TENSORFLOW_LITE_MICRO_EXAMPLES_FOOD_MENU_RECOGNITION_MODEL_SETTINGS_H_
/* Modified by Renesas Electronics } */
