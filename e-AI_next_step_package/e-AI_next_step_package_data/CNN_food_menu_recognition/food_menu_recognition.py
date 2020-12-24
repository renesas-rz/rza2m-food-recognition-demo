#!/usr/bin/env python
# coding: utf-8

# In[1]:

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import os

import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense
from tensorflow.keras.layers import Dropout
from tensorflow.keras.layers import BatchNormalization
from tensorflow.keras.layers import Input
from tensorflow.keras.layers import Activation
from tensorflow.keras.layers import Conv2D
from tensorflow.keras.layers import Flatten
from tensorflow.keras.layers import Reshape
from tensorflow.keras.layers import LeakyReLU
from tensorflow.keras.layers import MaxPooling2D
from tensorflow.keras.layers import ELU
from tensorflow.keras.layers import GlobalAveragePooling2D
from tensorflow.keras.layers import AveragePooling2D
from tensorflow.keras.models import Model
from tensorflow.keras.optimizers import Adam
from tensorflow.keras.callbacks import EarlyStopping
import tensorflow.keras.backend as K

#from sklearn.model_selection import train_test_split

from tensorflow.keras.preprocessing.image import ImageDataGenerator
from tensorflow.keras.preprocessing       import image
from PIL import Image, ImageOps

import tensorflow.keras as keras

# In[2]:

label_list = ['caesar_salad','club_sandwich','donuts',
 'edamame','french_fries','fried_rice','hamburger',
 'hot_dog','oysters',
 'pancakes','pizza','ramen',
 'spaghetti_carbonara','steak','sushi']

# In[3]:

data_height, data_width, data_channel = 128, 128, 3
class_num  = len(label_list)

print(class_num)

# Training

train_data_dir = 'data_15/train/'
valid_data_dir = 'data_15/valid/'

train_data_num = 12000
valid_data_num =  3000

test_data_num  =  1000
epoch_num      =  100
batch_size     =   400
augment_factor =     1

# In[4]:

train_datagen = ImageDataGenerator(
    rescale             = 1.0 / 255,
    horizontal_flip     = True,
    vertical_flip       = True,  
    rotation_range      = 20,
    height_shift_range  = 0.2,
    width_shift_range   = 0.2,
    zoom_range          = 0.2,
    brightness_range    = [0.3, 1.0],
    channel_shift_range = 5.0)

valid_datagen  = ImageDataGenerator(
    rescale             = 1.0 / 255)

test_datagen  = ImageDataGenerator(
    rescale             = 1.0 / 255)

train_generator = train_datagen.flow_from_directory(
    train_data_dir,
    classes = label_list,
    target_size = (data_height, data_width),
    batch_size = batch_size,
    class_mode = 'categorical')

valid_generator = valid_datagen.flow_from_directory(
    valid_data_dir,
    classes = label_list,
    target_size = (data_height, data_width),
    batch_size = batch_size,
    class_mode = 'categorical')

test_generator = test_datagen.flow_from_directory(
    valid_data_dir,
    classes = label_list,
    target_size = (data_height, data_width),
    batch_size = test_data_num,
    class_mode = 'categorical')

# In[5]:

train_sample_image, train_sample_label = next(train_generator)
valid_sample_image, valid_sample_label = next(valid_generator)


# In[6]:

def plotImages_train(images_arr, labels_arr):
    fig, axes = plt.subplots(4, 5, figsize=(14,14))
    axes = axes.flatten()
    for img, ax, lvl in zip( images_arr, axes, labels_arr):
        ax.imshow(img)
        ax.axis('off')
        ax.set(title = label_list[np.argmax(lvl)])
#   plt.tight_layout()
    plt.subplots_adjust(hspace=0.0001, wspace=0.05 )
    plt.savefig('./train_data.png')
    plt.show()

def plotImages_valid(images_arr, labels_arr):
    fig, axes = plt.subplots(4, 5, figsize=(14,14))
    axes = axes.flatten()
    for img, ax, lvl in zip( images_arr, axes, labels_arr):
        ax.imshow(img)
        ax.axis('off')
        ax.set(title = label_list[np.argmax(lvl)])
#   plt.tight_layout()
    plt.subplots_adjust(hspace=0.0001, wspace=0.05 )
    plt.savefig('./valid_data.png')
    plt.show()	

def plotImages_test(images_arr, labels_arr):
    fig, axes = plt.subplots(4, 5, figsize=(14,14))
    axes = axes.flatten()
    for img, ax, lvl in zip( images_arr, axes, labels_arr):
        ax.imshow(img)
        ax.axis('off')
        ax.set(title = label_list[np.argmax(lvl)])
#   plt.tight_layout()
    plt.subplots_adjust(hspace=0.0001, wspace=0.05 )
    plt.savefig('./test_data.png')
    plt.show()	

# In[7]:

plotImages_train(train_sample_image[:32], train_sample_label[:32])

# In[8]:

plotImages_valid(valid_sample_image[:32],  valid_sample_label[:32])


# In[9]:

model = Sequential()

model.add(Conv2D(8,  (3, 3), padding = 'same', activation = 'relu', input_shape = (data_height, data_width, data_channel)))
model.add(Conv2D(8,  (3, 3), padding = 'same', activation = 'relu'))
model.add(Conv2D(16, (3, 3), padding = 'same', activation = 'relu'))
model.add(AveragePooling2D(pool_size = (2, 2)))
model.add(Dropout(0.2))
#model.add(BatchNormalization())

#model.add(Conv2D(16, (3, 3), padding = 'same', activation = 'relu'))
model.add(Conv2D(16, (3, 3), padding = 'same', activation = 'relu'))
model.add(Conv2D(16, (3, 3), padding = 'same', activation = 'relu'))
model.add(Conv2D(32, (3, 3), padding = 'same', activation = 'relu'))
model.add(AveragePooling2D(pool_size = (2, 2)))
model.add(Dropout(0.2))
model.add(BatchNormalization())

model.add(Conv2D(32, (3, 3), padding = 'same', activation = 'relu'))
model.add(Conv2D(32, (3, 3), padding = 'same', activation = 'relu'))
model.add(Conv2D(32, (3, 3), padding = 'same', activation = 'relu'))
model.add(Conv2D(32, (3, 3), padding = 'same', activation = 'relu'))
model.add(AveragePooling2D(pool_size = (2, 2)))
model.add(Dropout(0.2))
#model.add(BatchNormalization())

model.add(Conv2D(64, (3, 3), padding = 'same', activation = 'relu'))
model.add(AveragePooling2D(pool_size = (2, 2)))
model.add(Dropout(0.2))
#model.add(BatchNormalization())

model.add(Flatten())

model.add(Dense(128, activation = 'relu'))
model.add(Dropout(0.2))

model.add(Dense(class_num, activation = 'softmax'))

model.summary()

# In[10]:

model.compile(optimizer="adam",loss="categorical_crossentropy",metrics=["accuracy"])

history = model.fit(
    train_generator,
    steps_per_epoch =  train_data_num // batch_size,
    epochs = epoch_num,
    validation_data = valid_generator,
    validation_steps = valid_data_num // batch_size)

# In[11]:

plt.figure(figsize = (18, 6))
plt.subplot(1, 2, 1)
plt.plot(history.history['accuracy'],marker = "o")
plt.plot(history.history['val_accuracy'], marker = "o")
plt.title('model accuracy', fontsize = 21)
plt.ylabel('accuracy', fontsize = 21)
plt.xlabel('epoch', fontsize = 21)
plt.legend(['train', 'validation'], loc = 'lower right')
plt.grid(color = 'gray', alpha = 0.2)
plt.subplot(1, 2, 2)
plt.plot(history.history['loss'], marker = "o")
plt.plot(history.history['val_loss'], marker = "o")
plt.title('model loss', fontsize = 21)
plt.ylabel('loss',fontsize = 21)
plt.xlabel('epoch',fontsize = 21)
plt.legend(['train', 'validation'], loc = 'upper right')
plt.grid(color = 'gray', alpha = 0.2)
plt.savefig('./figure.png')
plt.savefig('figure.png')
plt.show('figure.png')

# In[12]:

model.save('./food_menu_recognition.h5')

# In[13]:

def representative_dataset_gen():
    for i in range(10):
        data_x, data_y = train_generator.next()
#       plotImages(data_x[:32], data_y[:32])
#       data_x = data_x[0]
        for data_xx in data_x:
#           plt.imshow(data_xx)
#           plt.show()
            data_xx = tf.reshape(data_xx, shape=[-1, data_height, data_width, data_channel])
            yield [data_xx]

converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.representative_dataset = representative_dataset_gen
converter.target_spec.supported_ops =[tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter.inference_input_tpye  = tf.int8
converter.inference_output_tpye = tf.int8
food_menu_recognition = converter.convert()
open("./food_menu_recognition.tflite", "wb").write(food_menu_recognition)

# In[14]:

interpreter = tf.lite.Interpreter(model_path="./food_menu_recognition.tflite")
interpreter.allocate_tensors()
td = interpreter.get_tensor_details()

print("TFLITE model tensors (mint8)")
for i in range(len(td)):
    print("{:3d} : {:55s} {:14s} {:7s} {:s}".format(td[i]["index"], td[i]["name"], str(td[i]["shape"]), str(td[i]["dtype"]).replace("<class 'numpy.", "").replace("'>", ""), str(td[i]["quantization"])))
print()


# In[15]:

test_sample_image, test_sample_label = next(test_generator)

# In[16]:

plotImages_test(test_sample_image[:32],  test_sample_label[:32])

# In[17]:

model = tf.keras.models.load_model('./food_menu_recognition.h5')
test_pred = model.predict(test_sample_image)

print(test_pred)
results = test_pred.argmax(axis=1)

accurate_count = 0
for i, (res_label, test_label) in enumerate(zip(results, test_sample_label)):
    correct_label = np.argmax(test_label)
    accurate_count += (res_label == correct_label)
    
print(accurate_count / test_sample_label.shape[0])
print(i)

# In[18]:

interpreter_mquant = tf.lite.Interpreter(model_path="./food_menu_recognition.tflite")
interpreter_mquant.allocate_tensors()
input_details  = interpreter_mquant.get_input_details()
output_details = interpreter_mquant.get_output_details()

accurate_count = 0
ii=0
for image, label in zip( test_sample_image[:test_data_num], test_sample_label[:test_data_num]):
#    plt.imshow(image)
#    plt.show()
    image = tf.reshape(image, shape=[-1, data_height, data_width, data_channel])
    interpreter_mquant.set_tensor(input_details[0]['index'], image)
    interpreter_mquant.invoke()
    output_data = interpreter_mquant.get_tensor(output_details[0]['index'])
    if ii<25 :
      print(output_data)
    ii = ii+1
    predict_label = np.argmax(output_data)
    correct_label = np.argmax(label)
    accurate_count += (predict_label == correct_label)

print(accurate_count)
print(ii)

accuracy_mquant = accurate_count * 1.0 / test_data_num
print('TensorFlow Lite mquant model accuracy = %.4f \n' % accuracy_mquant)

