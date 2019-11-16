//
// Copyright 2019 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#ifndef PXR_USD_PCP_DYNAMIC_FILE_FORMAT_DEPENDENCY_DATA_H
#define PXR_USD_PCP_DYNAMIC_FILE_FORMAT_DEPENDENCY_DATA_H

#include "pxr/pxr.h"
#include "pxr/usd/pcp/api.h"
#include "pxr/base/tf/declarePtrs.h"
#include "pxr/base/tf/token.h"

#include <memory>
#include <vector>

PXR_NAMESPACE_OPEN_SCOPE

class PcpDynamicFileFormatInterface;
class VtValue;

TF_DECLARE_WEAK_PTRS(SdfFileFormat);

/// \class PcpDynamicFileFormatDependencyData
///
/// Contains the necessary information for storing a prim index's dependency
/// on dynamic file format arguments and determining if a field change affects
/// the prim index. This data structure does not store the prim index or its 
/// path itself and is expected to be the data in some other data structure
/// that maps prim indexes to its dependencies.
///
class PcpDynamicFileFormatDependencyData
{
public:
    /// Default constructor. This data will be empty.
    PCP_API
    PcpDynamicFileFormatDependencyData() = default;

    /// Move constructor.
    PCP_API
    PcpDynamicFileFormatDependencyData(
        PcpDynamicFileFormatDependencyData &&) = default;

    /// Copy constructor.
    PCP_API
    PcpDynamicFileFormatDependencyData(
        const PcpDynamicFileFormatDependencyData &rhs);

    /// Move assignment operator
    PcpDynamicFileFormatDependencyData &operator=(
        PcpDynamicFileFormatDependencyData &&rhs) {
        Swap(rhs);
        return *this;
    }

    /// Copy assignment operator
    PcpDynamicFileFormatDependencyData &operator=(
        const PcpDynamicFileFormatDependencyData &rhs) {
        PcpDynamicFileFormatDependencyData(rhs).Swap(*this);
        return *this;
    }

    /// Swap the contents of this dependency data with \p rhs.
    inline void Swap(PcpDynamicFileFormatDependencyData& rhs) {
        _data.swap(rhs._data);
    }

    /// Same as Swap(), but standard name.
    inline void swap(PcpDynamicFileFormatDependencyData &rhs) { Swap(rhs); }

    /// Returns whether this dependency data is empty.
    inline bool IsEmpty() const {
        return !_data;
    }

    /// Adds dependency info from a single context that generated dynamic file
    /// format arguments (usually a payload arc in the graph).  
    /// \p dynamicFileFormat is the file format that generated the arguments.
    /// \p dependencyContextData is custom dependency information generated when
    /// the file format generated its arguments. \p composedFieldNames is a 
    /// list of the fields that were composed on the prim to generate arguments.
    PCP_API
    void AddDependencyContext(
        const PcpDynamicFileFormatInterface *dynamicFileFormat,
        VtValue &&dependencyContextData,
        TfToken::Set &&composedFieldNames);

    /// Takes all the dependency data from \p dependencyData and adds it to this
    /// dependency.
    PCP_API
    void AppendDependencyData(
        PcpDynamicFileFormatDependencyData &&dependencyData);

    /// Returns a list of field names that were composed for any of the 
    /// dependency contexts that were added to this dependency.
    PCP_API
    const TfToken::Set &GetRelevantFieldNames() const;

    /// Given a \p field name and the changed field values in \p oldAndNewValues
    /// this return whether this change can affect any of the file format 
    /// arguments generated by any of the contexts stored in this dependency.
    PCP_API
    bool CanFieldChangeAffectFileFormatArguments(const TfToken &fieldName, 
                                                 const VtValue& oldValue,
                                                 const VtValue& newValue) const;

private:
    // Struct containing the entire contents of the dependency.
    struct _Data
    {
        using _ContextData = 
            std::pair<const PcpDynamicFileFormatInterface *, VtValue>;
        using _ContextDataVector = std::vector<_ContextData>;

        _ContextDataVector dependencyContexts;
        TfToken::Set relevantFieldNames;

        // Helper for adding relevant fields. We avoid copying by taking the 
        // input set if our set is empty.
        void _AddRelevantFieldNames(TfToken::Set &&fieldNames);
    };

    // Pointer to data. Will be null if this an empty data object.
    std::unique_ptr<_Data> _data;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_USD_PCP_DYNAMIC_FILE_FORMAT_DEPENDENCY_DATA_H