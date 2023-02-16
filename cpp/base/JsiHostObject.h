#pragma once

#include <jsi/jsi.h>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#define STR_CAT_NX(A, B) A##B
#define STR_CAT(A, B) STR_CAT_NX(A, B)
#define STR_GET get_
#define STR_SET set_

/**
 * Creates a new Host function declaration as a lambda with all deps passed
 * with implicit lambda capture clause
 */
#define JSI_HOST_FUNCTION_LAMBDA                                               \
  [=](jsi::Runtime & runtime, const jsi::Value &thisValue,                     \
      const jsi::Value *arguments, size_t count) -> jsi::Value

/**
 * Creates a new Host function declaration
 */
#define JSI_HOST_FUNCTION(NAME)                                                \
  jsi::Value NAME(jsi::Runtime &runtime, const jsi::Value &thisValue,          \
                  const jsi::Value *arguments, size_t count)

/**
 * Creates a new property setter function declaration
 */
#define JSI_PROPERTY_SET(NAME)                                                 \
  void STR_CAT(STR_SET, NAME)(jsi::Runtime & runtime, const jsi::Value &value)

/**
 * Creates a new property getter function declaration
 */
#define JSI_PROPERTY_GET(NAME)                                                 \
  jsi::Value STR_CAT(STR_GET, NAME)(jsi::Runtime & runtime)

/**
 * Creates a JSI export function declaration
 */
#define JSI_EXPORT_FUNC(CLASS, FUNCTION)                                       \
  {                                                                            \
#FUNCTION, (jsi::Value(JsiHostObject::*)(                                  \
                   jsi::Runtime & runtime, const jsi::Value &thisValue,        \
                   const jsi::Value *arguments, size_t)) &                     \
                   CLASS::FUNCTION                                             \
  }

/**
 * Creates a JSI export function declaration with a specific name
 */
#define JSI_EXPORT_FUNC_NAMED(CLASS, FUNCTION, NAME)                           \
  {                                                                            \
#NAME, (jsi::Value(JsiHostObject::*)(                                      \
               jsi::Runtime & runtime, const jsi::Value &thisValue,            \
               const jsi::Value *arguments, size_t)) &                         \
               CLASS::FUNCTION                                                 \
  }

/**
 * Creates a JSI export functions statement
 */
#define JSI_EXPORT_FUNCTIONS(...)                                              \
  const RNWorklet::JsiFunctionMap &getExportedFunctionMap() override {         \
    static RNWorklet::JsiFunctionMap map = {__VA_ARGS__};                      \
    return map;                                                                \
  }

/**
 * Creates a JSI export getter declaration
 */
#define JSI_EXPORT_PROP_GET(CLASS, FUNCTION)                                   \
  {                                                                            \
#FUNCTION, (jsi::Value(JsiHostObject::*)(jsi::Runtime & runtime)) &        \
                   CLASS::STR_CAT(STR_GET, FUNCTION)                           \
  }

/**
 * Creates a JSI export getters statement
 */
#define JSI_EXPORT_PROPERTY_GETTERS(...)                                       \
  const RNWorklet::JsiPropertyGettersMap &getExportedPropertyGettersMap()      \
      override {                                                               \
    static RNWorklet::JsiPropertyGettersMap map = {__VA_ARGS__};               \
    return map;                                                                \
  }

/**
 * Creates a JSI export setter declaration
 */
#define JSI_EXPORT_PROP_SET(CLASS, FUNCTION)                                   \
  {                                                                            \
#FUNCTION,                                                                 \
        (void(JsiHostObject::*)(jsi::Runtime & runtime, const jsi::Value &)) & \
            CLASS::STR_CAT(STR_SET, FUNCTION)                                  \
  }

/**
 * Creates a JSI export setters statement
 */
#define JSI_EXPORT_PROPERTY_SETTERS(...)                                       \
  const RNWorklet::JsiPropertySettersMap &getExportedPropertySettersMap()      \
      override {                                                               \
    static RNWorklet::JsiPropertySettersMap map = {__VA_ARGS__};               \
    return map;                                                                \
  }

namespace RNWorklet {

namespace jsi = facebook::jsi;

using JsPropertyType = struct {
  std::function<jsi::Value(jsi::Runtime &)> get;
  std::function<void(jsi::Runtime &, const jsi::Value &)> set;
};

class JsiHostObject;

using JsiFunctionMap =
    std::unordered_map<std::string, jsi::Value (JsiHostObject::*)(
                                        jsi::Runtime &, const jsi::Value &,
                                        const jsi::Value *, size_t)>;

using JsiPropertyGettersMap =
    std::unordered_map<std::string,
                       jsi::Value (JsiHostObject::*)(jsi::Runtime &)>;

using JsiPropertySettersMap =
    std::unordered_map<std::string, void (JsiHostObject::*)(
                                        jsi::Runtime &, const jsi::Value &)>;

/**
 * Base class for jsi host objects
 */
class JsiHostObject : public jsi::HostObject {
public:
  JsiHostObject();
  ~JsiHostObject();

protected:
  /**
   Override to return map of name/functions
   */
  virtual const RNWorklet::JsiFunctionMap &getExportedFunctionMap() {
    static const RNWorklet::JsiFunctionMap empty;
    return empty;
  }

  /**
   Override to get property getters map of name/functions
   */
  virtual const JsiPropertyGettersMap &getExportedPropertyGettersMap() {
    static const JsiPropertyGettersMap empty;
    return empty;
  }

  /**
   Override to get property setters map of name/functions
   */
  virtual const JsiPropertySettersMap &getExportedPropertySettersMap() {
    static const JsiPropertySettersMap empty;
    return empty;
  }

  /**
   * Overridden jsi::HostObject set property method
   * @param rt Runtime
   * @param name Name of value to set
   * @param value Value to set
   */
  void set(jsi::Runtime &rt, const jsi::PropNameID &name,
           const jsi::Value &value) override;

  /**
   * Overridden jsi::HostObject get property method. Returns functions from
   * the map of functions.
   * @param runtime Runtime
   * @param name Name of value to get
   * @return Value
   */
  jsi::Value get(jsi::Runtime &runtime, const jsi::PropNameID &name) override;

  /**
   * Overridden getPropertyNames from jsi::HostObject. Returns all keys in the
   * function and property maps
   * @param runtime Runtime
   * @return List of property names
   */
  std::vector<jsi::PropNameID> getPropertyNames(jsi::Runtime &runtime) override;

private:
  std::map<void *, std::map<std::string, jsi::Function>> _hostFunctionCache;
};
} // namespace RNWorklet
