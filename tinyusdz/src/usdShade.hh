// SPDX-License-Identifier: Apache 2.0
// Copyright 2022 - 2023, Syoyo Fujita.
// Copyright 2023 - Present, Light Transport Entertainment Inc.
//
// Material and Shader. And more, TinyUSDZ implmenents some usdImaging stuff here.
//
// TODO:
//   - [ ] Consider `interfaceOnly` connection
//   - [ ] Strict usdShade interpretation https://graphics.pixar.com/usd/release/api/usd_shade_page_front.html
//   - [ ] MaterialX support(in usdMtlx.hh)
//   - [ ] NodeGraph support
//
#pragma once

#include "prim-types.hh"

namespace tinyusdz {

constexpr auto kMaterial = "Material";
constexpr auto kShader = "Shader";
constexpr auto kNodeGraph = "NodeGraph";
constexpr auto kShaderNode = "ShaderNode";

constexpr auto kShaderInfoId = "info:id";

constexpr auto kUsdPreviewSurface = "UsdPreviewSurface";
constexpr auto kUsdUVTexture = "UsdUVTexture";
constexpr auto kUsdTransform2d = "UsdTransform2d";
constexpr auto kUsdPrimvarReader_int = "UsdPrimvarReader_int";
constexpr auto kUsdPrimvarReader_float = "UsdPrimvarReader_float";
constexpr auto kUsdPrimvarReader_float2 = "UsdPrimvarReader_float2";
constexpr auto kUsdPrimvarReader_float3 = "UsdPrimvarReader_float3";
constexpr auto kUsdPrimvarReader_float4 = "UsdPrimvarReader_float4";
constexpr auto kUsdPrimvarReader_string = "UsdPrimvarReader_string";
constexpr auto kUsdPrimvarReader_normal = "UsdPrimvarReader_normal";
constexpr auto kUsdPrimvarReader_point = "UsdPrimvarReader_point";
constexpr auto kUsdPrimvarReader_vector = "UsdPrimvarReader_vector";
constexpr auto kUsdPrimvarReader_matrix = "UsdPrimvarReader_matrix";

// for bindMaterialAs
constexpr auto kWeaderThanDescendants = "weakerThanDescendants";
constexpr auto kStrongerThanDescendants = "strongerThanDescendants";

enum class MaterialBindingStrength
{
  WeakerThanDescendants, // default
  StrongerThanDescendants
};

// TODO: Move to pprinter.hh?
std::string to_string(const MaterialBindingStrength strength);

class MaterialBinding {
 public:

  static value::token kAllPurpose() {
    return value::token("");
  }

  //
  // NOTE on material binding.
  // https://openusd.org/release/wp_usdshade.html
  //
  //  - "all purpose", direct binding, material:binding. single relationship target only
  //  - a purpose-restricted, direct, fallback binding, e.g. material:binding:preview
  //  - an all-purpose, collection-based binding, e.g. material:binding:collection:metalBits
  //  - a purpose-restricted, collection-based binding, e.g. material:binding:collection:full:metalBits
  //
  // In TinyUSDZ, treat empty purpose token as "all purpose"
  //

  // Some frequently used materialBindings
  nonstd::optional<Relationship> materialBinding; // material:binding
  nonstd::optional<Relationship> materialBindingCollection; // material:binding:collection  TODO: deprecate?(seems `material:binding:collection` without leaf NAME seems ignored in pxrUSD.
  nonstd::optional<Relationship> materialBindingPreview; // material:binding:preview
  nonstd::optional<Relationship> materialBindingFull; // material:binding:full

  value::token get_materialBindingStrength(const value::token &purpose);
  value::token get_materialBindingStrengthCollection(const value::token &collection_name, const value::token &purpose);


  bool has_materialBinding() const {
    return materialBinding.has_value();
  }

  bool has_materialBindingPreview() const {
    return materialBindingPreview.has_value();
  }

  bool has_materialBindingFull() const {
    return materialBindingFull.has_value();
  }

  bool has_materialBinding(const value::token &mat_purpose) const {
    if (mat_purpose.str() == kAllPurpose().str()) {
      return has_materialBinding();
    } else if (mat_purpose.str() == "full") {
      return has_materialBindingFull();
    } else if (mat_purpose.str() == "preview") {
      return has_materialBindingPreview();
    } else {
      return _materialBindingMap.count(mat_purpose.str());
    }
  }

  void clear_materialBinding() {
    materialBinding.reset();
  }

  void clear_materialBindingPreview() {
    materialBindingPreview.reset();
  }

  void clear_materialBindingFull() {
    materialBindingFull.reset();
  }

  void set_materialBinding(const Relationship &rel) {
    materialBinding = rel;
  }

  void set_materialBinding(const Relationship &rel, const MaterialBindingStrength strength) {
    value::token strength_tok(to_string(strength));
    materialBinding = rel;
    materialBinding.value().metas().bindMaterialAs = strength_tok;
  }

  void set_materialBindingPreview(const Relationship &rel) {
    materialBindingPreview = rel;
  }

  void set_materialBindingPreview(const Relationship &rel, const MaterialBindingStrength strength) {
    value::token strength_tok(to_string(strength));
    materialBindingPreview = rel;
    materialBindingPreview.value().metas().bindMaterialAs = strength_tok;
  }

  void set_materialBindingFull(const Relationship &rel) {
    materialBindingFull = rel;
  }

  void set_materialBindingFull(const Relationship &rel, const MaterialBindingStrength strength) {
    value::token strength_tok(to_string(strength));
    materialBindingFull = rel;
    materialBindingFull.value().metas().bindMaterialAs = strength_tok;
  }

  void set_materialBinding(const Relationship &rel, const value::token &mat_purpose) {

    if (mat_purpose.str().empty()) {
      return set_materialBinding(rel);
    } else if (mat_purpose.str() == "full") {
      return set_materialBindingFull(rel);
    } else if (mat_purpose.str() == "preview") {
      return set_materialBindingFull(rel);
    } else {
      _materialBindingMap[mat_purpose.str()] = rel;
    }
  }

  void set_materialBinding(const Relationship &rel, const value::token &mat_purpose, const MaterialBindingStrength strength) {
    value::token strength_tok(to_string(strength));

    if (mat_purpose.str().empty()) {
      return set_materialBinding(rel, strength);
    } else if (mat_purpose.str() == "full") {
      return set_materialBindingFull(rel, strength);
    } else if (mat_purpose.str() == "preview") {
      return set_materialBindingFull(rel, strength);
    } else {
      _materialBindingMap[mat_purpose.str()] = rel;
      _materialBindingMap[mat_purpose.str()].metas().bindMaterialAs = strength_tok;
    }
  }

  bool has_materialBindingCollection(const std::string &tok) {

    if (!_materialBindingCollectionMap.count(tok)) {
      return false;
    }

    return _materialBindingCollectionMap.count(tok);
  }

  void set_materialBindingCollection(const value::token &tok, const value::token &mat_purpose, const Relationship &rel) {

    // NOTE:
    // https://openusd.org/release/wp_usdshade.html#basic-proposal-for-collection-based-assignment
    // says: material:binding:collection defines a namespace of binding relationships to be applied in namespace order, with the earliest ordered binding relationship the strongest
    //
    // so the app is better first check if `tok` element alreasy exists(using has_materialBindingCollection)

    auto &m = _materialBindingCollectionMap[tok.str()];

    m[mat_purpose.str()] = rel;
  }

  void clear_materialBindingCollection(const value::token &tok, const value::token &mat_purpose) {
    if (_materialBindingCollectionMap.count(tok.str())) {
      _materialBindingCollectionMap[tok.str()].erase(mat_purpose.str());
    }
  }

  void set_materialBindingCollection(const value::token &tok, const value::token &mat_purpose, const Relationship &rel, MaterialBindingStrength strength) {
    value::token strength_tok(to_string(strength));

    _materialBindingCollectionMap[tok.str()][mat_purpose.str()] = rel;
    _materialBindingCollectionMap[tok.str()][mat_purpose.str()].metas().bindMaterialAs = strength_tok;

  }

  const std::map<std::string, std::map<std::string, Relationship>> materialBindingCollectionMap() const {
    return _materialBindingCollectionMap;
  }

 private:

  // For material:binding(excludes frequently used `material:binding`, `material:binding:full` and `material:binding:preview`)
  // key = PURPOSE, value = rel
  std::map<std::string, Relationship> _materialBindingMap;

  // For material:binding:collection
  // key = NAME, value = map<PURPOSE, Rel>
  // TODO: Use multi-index map
  std::map<std::string, std::map<std::string, Relationship>> _materialBindingCollectionMap;
};

// TODO: Inherit from Prim?
struct UsdShadePrim {
  std::string name;
  Specifier spec{Specifier::Def};

  int64_t parent_id{-1};

  PrimMeta meta; // TODO: move to private

  const PrimMeta &metas() const { return meta; }
  PrimMeta &metas() { return meta; }

  // Check if `key` exists in `sdrMetadata` metadatum.
  // Return false when `key` is not found in `sdrMetadata`, or corrensponding item is not a string type.
  bool has_sdr_metadata(const std::string &key);

  // Get value from `sdrMetadata` metadatum.
  // Return empty string when `key` is not found in `sdrMetadata`, or corrensponding item is not a string type.
  const std::string get_sdr_metadata(const std::string &key);

  // Set value to `sdrMetadata` metadatum.
  // Return false when error(e.g. `key` contains invalid character for USD dictionary)
  bool set_sdr_metadata(const std::string &key, const std::string &value);

  TypedAttributeWithFallback<Purpose> purpose{
      Purpose::Default};  // "uniform token purpose"

  std::pair<ListEditQual, std::vector<Reference>> references;
  std::pair<ListEditQual, std::vector<Payload>> payload;
  std::map<std::string, VariantSet> variantSet;
  // Custom properties
  std::map<std::string, Property> props;

  const std::vector<value::token> &primChildrenNames() const { return _primChildren; }
  const std::vector<value::token> &propertyNames() const { return _properties; }
  std::vector<value::token> &primChildrenNames() { return _primChildren; }
  std::vector<value::token> &propertyNames() { return _properties; }

 private:
  std::vector<value::token> _primChildren;
  std::vector<value::token> _properties;
};

//
// Similar to Maya's ShadingGroup
//
struct Material : UsdShadePrim {

  ///
  /// NOTE: Mateiral's outputs must be a connection.
  /// (Whereas Shader's outputs is not)
  ///
  TypedConnection<value::token> surface; // "token outputs:surface.connect"
  TypedConnection<value::token> displacement; // "token outputs:displacement.connect"
  TypedConnection<value::token> volume; // "token outputs:volume.connect"


};

// TODO
struct NodeGraph : UsdShadePrim {

};

//
// Base class of ShaderNode. Maybe similar to SdrShaderNode in pxrUSD
//
struct ShaderNode : UsdShadePrim {

};

template <typename T>
struct UsdPrimvarReader : ShaderNode {

  TypedAttribute<Animatable<T>> fallback;  // "inputs:fallback"

  TypedAttribute<Animatable<std::string>> varname;  // "string inputs:varname". Name of the primvar to be fetched from the geometry("primvar" namespace is omitted). NOTE: older spec uses `token` type: https://openusd.org/release/spec_usdpreviewsurface.html#version-2-3


  ///
  /// Outputs
  ///
  TypedTerminalAttribute<T> result; // Terminal attr. "T outputs:result"

};

using UsdPrimvarReader_float = UsdPrimvarReader<float>;
using UsdPrimvarReader_float2 = UsdPrimvarReader<value::float2>;
using UsdPrimvarReader_float3 = UsdPrimvarReader<value::float3>;
using UsdPrimvarReader_float4 = UsdPrimvarReader<value::float4>;
using UsdPrimvarReader_int = UsdPrimvarReader<int>;
using UsdPrimvarReader_string = UsdPrimvarReader<std::string>;

// The underlying type is float precision for `normal`, `vector` and `point`
using UsdPrimvarReader_normal = UsdPrimvarReader<value::normal3f>;
using UsdPrimvarReader_vector = UsdPrimvarReader<value::vector3f>;
using UsdPrimvarReader_point  = UsdPrimvarReader<value::point3f>;

// The underlying type is matrix4d
using UsdPrimvarReader_matrix  = UsdPrimvarReader<value::matrix4d>;

// TODO: Remove
//using UsdPrimvarReaderType =
//    tinyusdz::variant<UsdPrimvarReader_float, UsdPrimvarReader_float2,
//                      UsdPrimvarReader_float3, UsdPrimvarReader_float4,
//                      UsdPrimvarReader_int>;


struct UsdUVTexture : ShaderNode {

  enum class Wrap {
    UseMetadata, // "useMetadata" (default)
    Black, // "black"
    Clamp, // "clamp"
    Repeat, // "repeat"
    Mirror, // "mirror"
  };

  enum class SourceColorSpace {
    Auto, // "auto"(default)
    Raw, // "raw"
    SRGB, // "sRGB
  };

  TypedAttribute<Animatable<value::AssetPath>> file; // "asset inputs:file" interfaceOnly

  TypedAttributeWithFallback<Animatable<value::texcoord2f>> st{value::texcoord2f{0.0f, 0.0f}}; // "inputs:st"

  TypedAttributeWithFallback<Animatable<Wrap>> wrapS{Wrap::UseMetadata}; // "token inputs:wrapS" interfaceOnly
  TypedAttributeWithFallback<Animatable<Wrap>> wrapT{Wrap::UseMetadata}; // "token inputs:wrapT" interfaceOnly

  TypedAttributeWithFallback<value::color4f> fallback{{0.0f, 0.0f, 0.0f, 1.0f}}; // "inputs:fallback" Fallback value when no texture is connected(TODO: Disallow Relation?(i.e, `fallback.connect = </Path/To/FallbackColor>`)

  TypedAttributeWithFallback<Animatable<SourceColorSpace>> sourceColorSpace{SourceColorSpace::Auto}; // "token inputs:sourceColorSpace" interfaceOnly

  TypedAttributeWithFallback<value::float4> scale{{1.0f, 1.0f, 1.0f, 1.0f}}; // "inputs:scale" interfaceOnly
  TypedAttributeWithFallback<value::float4> bias{{0.0f, 0.0f, 0.0f, 0.0f}}; // "inputs:bias" interfaceOnly

  ///
  /// Outputs
  ///
  /// Terminal attribute.
  ///
  TypedTerminalAttribute<float> outputsR; // "float outputs:r"
  TypedTerminalAttribute<float> outputsG; // "float outputs:g"
  TypedTerminalAttribute<float> outputsB; // "float outputs:b"
  TypedTerminalAttribute<float> outputsA; // "float outputs:a"
  TypedTerminalAttribute<value::float3> outputsRGB; // "float outputs:rgb" in schema. Allow color3f as well(please use TypedTerminalAttribute::get_actual_type_name() to get a actual type name in USDA/USDC).

  // TODO: orientation?
  // https://graphics.pixar.com/usd/docs/UsdPreviewSurface-Proposal.html#UsdPreviewSurfaceProposal-TextureCoordinateOrientationinUSD
};

// UsdPreviewSurface
// USD's default? PBR shader
// https://graphics.pixar.com/usd/docs/UsdPreviewSurface-Proposal.html
// $USD/pxr/usdImaging/plugin/usdShaders/shaders/shaderDefs.usda

struct UsdPreviewSurface : ShaderNode {

  TypedAttributeWithFallback<Animatable<value::color3f>> diffuseColor{value::color3f{0.18f, 0.18f, 0.18f}};  // "inputs:diffuseColor"
  TypedAttributeWithFallback<Animatable<value::color3f>> emissiveColor{value::color3f{0.0f, 0.0f, 0.0f}};  // "inputs:emissiveColor"

  TypedAttributeWithFallback<Animatable<int>> useSpecularWorkflow{0}; // "inputs:useSpecularWorkflow"

  // specular workflow
  TypedAttributeWithFallback<Animatable<value::color3f>> specularColor{value::color3f{0.0f, 0.0f, 0.0f}};

  // metalness workflow
  //TypedAttributeWithFallback<float> metallic{0.0f};  // "inputs:metallic"
  TypedAttributeWithFallback<Animatable<float>> metallic{0.0f};  // "inputs:metallic"

  //
  TypedAttributeWithFallback<Animatable<float>> clearcoat{0.0f};  // "inputs:clearcoat"
  TypedAttributeWithFallback<Animatable<float>> clearcoatRoughness{0.01f};  // "inputs:clearcoatRouighness"
  TypedAttributeWithFallback<Animatable<float>> roughness{0.5f};  // "inputs:roughness"
  TypedAttributeWithFallback<Animatable<float>> opacity{1.0f};  // "inputs:opacity"
  TypedAttributeWithFallback<Animatable<float>> opacityThreshold{0.0f};  // "inputs:opacityThreshold"
  TypedAttributeWithFallback<Animatable<float>> ior{1.5f};  // "inputs:ior"

  TypedAttributeWithFallback<Animatable<value::normal3f>> normal{value::normal3f{0.0f, 0.0f, 1.0f}}; // "inputs:normal"
  TypedAttributeWithFallback<Animatable<float>> displacement{0.0f}; // "inputs:displacement"
  TypedAttributeWithFallback<Animatable<float>> occlusion{0.0f}; // "inputs:occlusion"

  ///
  /// Outputs
  ///
  /// No value assigned.
  ///
  TypedTerminalAttribute<value::token> outputsSurface; // "token outputs:surface"
  TypedTerminalAttribute<value::token> outputsDisplacement; // "token outputs:displacement"

};

// Transform texture coordinates.
struct UsdTransform2d : ShaderNode {

  TypedAttributeWithFallback<Animatable<value::float2>> in{value::float2{0.0f, 0.0f}};  // "inputs:in" Usually connected to UsdPrimvarReader_float2

  // Transform is TRS order:
  //
  // result = in * scale * rotate * translation (in USD's notation(row-major, pre-multiply matrix mul))
  // result = translation * rotate * scale * in (in OpenGL's notation(column-major, post-multiply matrix mul))

  TypedAttributeWithFallback<Animatable<float>> rotation{0.0f};  // "inputs:rotation" CCW, in degree.
  TypedAttributeWithFallback<Animatable<value::float2>> scale{value::float2{1.0f, 1.0f}};  // "inputs:scale"
  TypedAttributeWithFallback<Animatable<value::float2>> translation{value::float2{0.0f, 0.0f}};  // "inputs:translation"


  ///
  /// Outputs
  ///
  TypedTerminalAttribute<value::float2> result; // "float2 outputs:result"

};

// Shader Prim
struct Shader : UsdShadePrim {

  std::string info_id;  // ShaderNode type.

  // ShaderNode, UsdPreviewSurface, UsdUVTexture, UsdPrimvarReader_float2, ...
  // TODO: Use ShaderNode *?
  value::Value value;
#if 0
  // Currently we only support PreviewSurface, UVTexture and
  // PrimvarReader_float2
  tinyusdz::variant<tinyusdz::monostate, PreviewSurface, UVTexture,
                    PrimvarReader_float2>
      value;
#endif

};

// import DEFINE_TYPE_TRAIT and DEFINE_ROLE_TYPE_TRAIT
#include "define-type-trait.inc"

namespace value {

// Mateiral Prim
DEFINE_TYPE_TRAIT(Material, kMaterial,
                  TYPE_ID_MATERIAL, 1);

// Shader Prim
DEFINE_TYPE_TRAIT(Shader, kShader,
                  TYPE_ID_SHADER, 1);

// NodeGraph Prim
DEFINE_TYPE_TRAIT(NodeGraph, kNodeGraph,
                  TYPE_ID_NODEGRAPH, 1);


// ShaderNodes
DEFINE_TYPE_TRAIT(ShaderNode, kShaderNode,
                  TYPE_ID_IMAGING_SHADER_NODE, 1);
DEFINE_TYPE_TRAIT(UsdPreviewSurface, kUsdPreviewSurface,
                  TYPE_ID_IMAGING_PREVIEWSURFACE, 1);
DEFINE_TYPE_TRAIT(UsdUVTexture, kUsdUVTexture, TYPE_ID_IMAGING_UVTEXTURE, 1);
DEFINE_TYPE_TRAIT(UsdPrimvarReader_float, kUsdPrimvarReader_float,
                  TYPE_ID_IMAGING_PRIMVAR_READER_FLOAT, 1);
DEFINE_TYPE_TRAIT(UsdPrimvarReader_float2, kUsdPrimvarReader_float2,
                  TYPE_ID_IMAGING_PRIMVAR_READER_FLOAT2, 1);
DEFINE_TYPE_TRAIT(UsdPrimvarReader_float3, kUsdPrimvarReader_float3,
                  TYPE_ID_IMAGING_PRIMVAR_READER_FLOAT3, 1);
DEFINE_TYPE_TRAIT(UsdPrimvarReader_float4, kUsdPrimvarReader_float4,
                  TYPE_ID_IMAGING_PRIMVAR_READER_FLOAT4, 1);
DEFINE_TYPE_TRAIT(UsdPrimvarReader_int, kUsdPrimvarReader_int,
                  TYPE_ID_IMAGING_PRIMVAR_READER_INT, 1);
DEFINE_TYPE_TRAIT(UsdPrimvarReader_string, kUsdPrimvarReader_string,
                  TYPE_ID_IMAGING_PRIMVAR_READER_STRING, 1);
DEFINE_TYPE_TRAIT(UsdPrimvarReader_vector, kUsdPrimvarReader_vector,
                  TYPE_ID_IMAGING_PRIMVAR_READER_VECTOR, 1);
DEFINE_TYPE_TRAIT(UsdPrimvarReader_normal, kUsdPrimvarReader_normal,
                  TYPE_ID_IMAGING_PRIMVAR_READER_NORMAL, 1);
DEFINE_TYPE_TRAIT(UsdPrimvarReader_point, kUsdPrimvarReader_point,
                  TYPE_ID_IMAGING_PRIMVAR_READER_POINT, 1);
DEFINE_TYPE_TRAIT(UsdPrimvarReader_matrix, kUsdPrimvarReader_matrix,
                  TYPE_ID_IMAGING_PRIMVAR_READER_MATRIX, 1);
DEFINE_TYPE_TRAIT(UsdTransform2d, kUsdTransform2d,
                  TYPE_ID_IMAGING_TRANSFORM_2D, 1);

DEFINE_TYPE_TRAIT(MaterialBinding, "MaterialBindingAPI",
                  TYPE_ID_MATERIAL_BINDING, 1);

#undef DEFINE_TYPE_TRAIT
#undef DEFINE_ROLE_TYPE_TRAIT

}  // namespace value

}  // namespace tinyusdz
