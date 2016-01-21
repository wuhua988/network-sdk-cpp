LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_LDLIBS += -llog
#LOCAL_CFLAGS := -DDEBUG

NDK_DEBUG=1
LOCAL_C_INCLUDES :=  $(LOCAL_PATH)/../src

#LOCAL_SHARED_LIBRARIES := $(LOCAL_PATH)/../android/libs/armeabi/libcrypto.so
#LOCAL_SHARED_LIBRARIES += $(LOCAL_PATH)/../android/libs/armeabi/libssl.so

LOCAL_LDLIBS += -L$(LOCAL_PATH)/../android/libs/armeabi/
LOCAL_LDLIBS += -L$(LOCAL_PATH)/../android/libs/armeabi-v7a/ 
LOCAL_LDLIBS += -L$(LOCAL_PATH)/../android/libs/x86/ 
LOCAL_LDLIBS +=  
#LOCAL_LDLIBS += $(LOCAL_PATH)/../android/libs/armeabi/libssl.so

LOCAL_MODULE   :=  network
MY_FILES_PATH  :=  $(LOCAL_PATH)/../src $(LOCAL_PATH)

MY_FILES_SUFFIX := %.cpp
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

MY_ALL_FILES := $(foreach src_path,$(MY_FILES_PATH), $(call rwildcard,$(src_path),*.*) )
MY_ALL_FILES := $(MY_ALL_FILES:$(MY_CPP_PATH)/./%=$(MY_CPP_PATH)%)
MY_SRC_LIST  := $(filter $(MY_FILES_SUFFIX),$(MY_ALL_FILES))
MY_SRC_LIST  := $(MY_SRC_LIST:$(LOCAL_PATH)/%=%)

define uniq =
  $(eval seen :=)
  $(foreach _,$1,$(if $(filter $_,${seen}),,$(eval seen += $_)))
  ${seen}
endef

MY_ALL_DIRS := $(dir $(foreach src_path,$(MY_FILES_PATH), $(call rwildcard,$(src_path),*/) ) )
MY_ALL_DIRS := $(call uniq,$(MY_ALL_DIRS))

LOCAL_SRC_FILES  := $(MY_SRC_LIST)
LOCAL_C_INCLUDES += $(MY_ALL_DIRS)

include $(BUILD_SHARED_LIBRARY)
