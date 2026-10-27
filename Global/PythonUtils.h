/* ***** BEGIN LICENSE BLOCK *****
 * This file is part of Natron <https://natrongithub.github.io/>,
 * (C) 2018-2023 The Natron developers
 * (C) 2013-2018 INRIA and Alexandre Gauthier-Foichat
 *
 * Natron is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Natron is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Natron.  If not, see <http://www.gnu.org/licenses/gpl-2.0.html>
 * ***** END LICENSE BLOCK ***** */

#ifndef NATRON_GLOBAL_PYTHONUTILS_H
#define NATRON_GLOBAL_PYTHONUTILS_H

// ***** BEGIN PYTHON BLOCK *****
// from <https://docs.python.org/3/c-api/intro.html#include-files>:
// "Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included."
#include <Python.h>
// ***** END PYTHON BLOCK *****

#include <string>
#include <vector>

#include "../Global/Macros.h"

#define PY_VERSION_STRINGIZE_(major, minor) \
# major "." # minor

#define PY_VERSION_STRINGIZE(major, minor) \
PY_VERSION_STRINGIZE_(major, minor)

#define NATRON_PY_VERSION_STRING PY_VERSION_STRINGIZE(PY_MAJOR_VERSION, PY_MINOR_VERSION)


#define PY_VERSION_STRINGIZE_NO_DOT_(major, minor) \
# major # minor

#define PY_VERSION_STRINGIZE_NO_DOT(major, minor) \
PY_VERSION_STRINGIZE_NO_DOT_(major, minor)

#define NATRON_PY_VERSION_STRING_NO_DOT PY_VERSION_STRINGIZE_NO_DOT(PY_MAJOR_VERSION, PY_MINOR_VERSION)

NATRON_NAMESPACE_ENTER

NATRON_PYTHON_NAMESPACE_ENTER

/**
 * @brief Calls Py_SetPythonHome and set PYTHONPATH
 **/
void setupPythonEnv(const std::string& binPath);

/**
 * @brief Must be called after setupPythonEnv(), calls Py_SetProgramName and Py_Initialize, PySys_SetArgv
 * @returns A pointer to the main module
 **/
PyObject* initializePython3(const std::vector<wchar_t*>& commandLineArgsWide);

std::string PyStringToStdString(PyObject* obj);

#if PY_VERSION_HEX >= 0x030F0000
/**
 * @brief Adapter for modules that only provide a PyModExport_<name> hook (what Shiboken >= 6.12 generates
 * for Python >= 3.15) but must be registered with PyImport_AppendInittab(), which only accepts
 * PyInit_<name>-style functions.
 * Call it from a PyInit-style function: it runs the export hook and builds a regular multi-phase
 * PyModuleDef (stored in @p def / @p slotStorage, which must be static) from the returned slots.
 **/
PyObject* moduleFromExportHook(PySlot* (*exportHook)(void),
                               const char* name,
                               PyModuleDef* def,
                               PyModuleDef_Slot* slotStorage,
                               size_t slotStorageSize);
#endif

NATRON_PYTHON_NAMESPACE_EXIT

NATRON_NAMESPACE_EXIT

#endif // ifndef NATRON_GLOBAL_PYTHONUTILS_H
