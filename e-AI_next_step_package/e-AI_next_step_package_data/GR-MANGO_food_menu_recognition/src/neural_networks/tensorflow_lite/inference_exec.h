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

#ifndef TENSORFLOW_LITE_MICRO_EXAMPLES_FOOD_MENU_RECOGNITION_MAIN_FUNCTIONS_H_
#define TENSORFLOW_LITE_MICRO_EXAMPLES_FOOD_MENU_RECOGNITION_MAIN_FUNCTIONS_H_

// Expose a C friendly interface for main functions.
#ifdef __cplusplus
extern "C" {
#endif

// Initializes all data needed for the example. The name is important, and needs
// to be setup() for Arduino compatibility.
//void setup();

// Runs one iteration of data gathering and inference. This should be called
// repeatedly from the application code. The name needs to be loop() for Arduino
// compatibility.
//void loop();

void inference_exec();
void inference_exec_init(void);

typedef struct
{
    uint32_t us_total;
    float    score;
    char     *category;
    float    caesar_salad_score;
    float    club_sandwich_score;
    float    donuts_score;
    float    edamame_score;
    float    french_fries_score;
    float    fried_rice_score;
    float    hamburger_score;
    float    hot_dog_score;
    float    oysters_score;
    float    pancakes_score;
    float    pizza_score;
    float    ramen_score;
    float    spaghetti_carbonara_score;
    float    steak_score;
    float    sushi_score;
    char	 *price;
}
inference_result_t;

#ifdef __cplusplus
}
#endif

#endif  // TENSORFLOW_LITE_MICRO_EXAMPLES_FOOD_MENU_RECOGNITION_MAIN_FUNCTIONS_H_
