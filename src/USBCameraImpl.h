/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2016. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#ifndef CAMERASERVER_USBCAMERAIMPL_H_
#define CAMERASERVER_USBCAMERAIMPL_H_

#include <atomic>
#include <thread>

#ifdef __linux__
#include <linux/videodev2.h>
#endif

#include "llvm/raw_ostream.h"
#include "llvm/DenseMap.h"
#include "llvm/SmallVector.h"
#include "llvm/StringMap.h"
#include "support/raw_istream.h"

#include "SourceImpl.h"

namespace cs {

class USBCameraImpl : public SourceImpl {
 public:
  USBCameraImpl(llvm::StringRef name, llvm::StringRef path);
  ~USBCameraImpl() override;

  llvm::StringRef GetDescription(
      llvm::SmallVectorImpl<char>& buf) const override;

  // Property functions
  int GetPropertyIndex(llvm::StringRef name) const override;
  llvm::ArrayRef<int> EnumerateProperties(
      llvm::SmallVectorImpl<int>& vec) const override;
  CS_PropertyType GetPropertyType(int property) const override;
  llvm::StringRef GetPropertyName(int property,
                                  llvm::SmallVectorImpl<char>& buf,
                                  CS_Status* status) const override;
  int GetProperty(int property, CS_Status* status) const override;
  void SetProperty(int property, int value, CS_Status* status) override;
  int GetPropertyMin(int property, CS_Status* status) const override;
  int GetPropertyMax(int property, CS_Status* status) const override;
  int GetPropertyStep(int property, CS_Status* status) const override;
  int GetPropertyDefault(int property, CS_Status* status) const override;
  llvm::StringRef GetStringProperty(int property,
                                    llvm::SmallVectorImpl<char>& buf,
                                    CS_Status* status) const override;
  void SetStringProperty(int property, llvm::StringRef value,
                         CS_Status* status) override;
  std::vector<std::string> GetEnumPropertyChoices(
      int property, CS_Status* status) const override;

  void Stop();

  struct PropertyData {
    PropertyData() = default;
#ifdef __linux__
#ifdef VIDIOC_QUERY_EXT_CTRL
    PropertyData(const struct v4l2_query_ext_ctrl& ctrl);
#endif
    PropertyData(const struct v4l2_queryctrl& ctrl);
#endif

    std::string name;
    unsigned id;  // implementation-level id
    int type;  // implementation type, not CS_PropertyType!
    CS_PropertyType propType;
    int minimum;
    int maximum;
    int step;
    int defaultValue;
  };

 private:
  mutable llvm::DenseMap<int, PropertyData> m_property_data;
  mutable llvm::StringMap<int> m_properties;
  mutable std::atomic_bool m_properties_cached{false};

  void CacheProperty(PropertyData&& prop) const;
  void CacheProperties() const;
  bool GetPropertyTypeValueFd(int property, int propType, unsigned* id,
                              int* type, int* fd, CS_Status* status) const;

  void CameraThreadMain();

  std::string m_path;
  std::string m_description;

  std::atomic_int m_fd;

  std::atomic_bool m_active;  // set to false to terminate threads
  std::thread m_cameraThread;

  mutable std::mutex m_mutex;
};

}  // namespace cs

#endif  // CAMERASERVER_USBCAMERAIMPL_H_
