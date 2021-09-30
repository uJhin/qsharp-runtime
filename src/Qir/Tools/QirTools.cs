﻿// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

using System;
using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;
using Microsoft.Quantum.Qir.Runtime.Tools.Executable;
using Microsoft.Quantum.QsCompiler;

namespace Microsoft.Quantum.Qir.Runtime.Tools
{
    /// <summary>
    /// Provides high-level utility methods to work with QIR.
    /// </summary>
    public static class QirTools
    {
        /// <summary>
        /// Creates QIR-based executables from a .NET DLL generated by the Q# compiler.
        /// </summary>
        /// <param name="qsharpDll">.NET DLL generated by the Q# compiler.</param>
        /// <param name="libraryDirectories">Directory where the libraries to link to are located.</param>
        /// <param name="includeDirectories">Directory where the headers needed for compilation are located.</param>
        /// <param name="executablesDirectory">Directory where the created executables are placed.</param>
        /// <param name="debug">Enable additional debugging checks at runtime.</param>
        public static async Task BuildFromQSharpDll(
            FileInfo qsharpDll,
            IList<DirectoryInfo> libraryDirectories,
            IList<DirectoryInfo> includeDirectories,
            DirectoryInfo executablesDirectory,
            bool debug)
        {
            using var qirContentStream = new MemoryStream();
            if (!AssemblyLoader.LoadQirBitcode(qsharpDll, qirContentStream))
            {
                throw new ArgumentException("The given DLL does not contain QIR bitcode.");
            }

            executablesDirectory.Create();

            // Build an executable for each entry point operation. The builds must run one at a time because they write
            // to the same intermediate files.
            var qirBitcode = qirContentStream.ToArray();
            foreach (var entryPointOp in EntryPointOperationLoader.LoadEntryPointOperations(qsharpDll.FullName))
            {
                var exeFileInfo = new FileInfo(Path.Combine(executablesDirectory.FullName, $"{entryPointOp.Name}.exe"));
                var exe = new QirFullStateExecutable(exeFileInfo, qirBitcode, debug);
                await exe.BuildAsync(entryPointOp, libraryDirectories, includeDirectories);
            }

            // ToDo: Return list of created file names
        }
    }
}
