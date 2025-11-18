[CmdletBinding()]
param ()

# This script builds the bit7z submodule located in 'third-party/bit7z'.
# It assumes you are running this script from the root of your repository.
#
# It builds the following matrix:
#   Platforms: Win32, x64
#   Configurations: Debug, Release
#
# Output Directories:
#   - Intermediate (.sln, .vcxproj, .obj): build/obj/$(Platform)/$(Configuration)
#

Write-Host "Starting bit7z build script..." -ForegroundColor Cyan

try {
    # --- Configuration ---
    $repoRoot = (Get-Location).Path
    $sourceDir = Join-Path -Path $repoRoot -ChildPath "third-party/bit7z"
    
    $platforms = "Win32", "x64"
    $configurations = "Debug", "Release"

    # --- Pre-flight Check ---
    if (-not (Test-Path -Path $sourceDir -PathType Container)) {
        Write-Error "Source directory not found: $sourceDir"
        Write-Error "Please ensure the submodule is initialized (e.g., 'git submodule update --init --recursive')"
        return # Exit the script
    }
    
    # Check if cmake is available
    $cmakeCheck = Get-Command cmake -ErrorAction SilentlyContinue
    if (-not $cmakeCheck) {
         Write-Error "CMake not found in your PATH. Please install CMake and ensure it's accessible."
         return
    }

    Write-Host "Repository Root: $repoRoot"
    Write-Host "bit7z Source:    $sourceDir"
    Write-Host ""

    # --- Build Loop ---
    foreach ($platform in $platforms) {
        foreach ($config in $configurations) {
            
            Write-Host "--------------------------------------------------"
            Write-Host "Processing: $platform / $config"
            Write-Host "--------------------------------------------------"

            # --- 1. Define Paths ---
            # Path for intermediate files (.sln, .vcxproj, .obj, etc.)
            $buildDir = Join-Path -Path $repoRoot -ChildPath "build/obj/bit7z/$platform/$config"
            
            # --- We no longer define or create $outputBinDir ---
            
            # Create directories if they don't exist
            if (-not (Test-Path -Path $buildDir)) {
                New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
            }
            
            # --- 2. Set Platform-Specific CMake Arg ---
            # For Visual Studio generators, -A "x64" is correct, and -A "Win32" is correct.
            $cmakePlatformArg = $platform

            # --- 3. Configure/Generate Step ---
            
            Write-Host "Running CMake Configuration..."
            $cmakeConfigArgs = @(
                "-S", $sourceDir,
                "-B", $buildDir,
                "-A", $cmakePlatformArg,
                "-DCMAKE_BUILD_TYPE=$config",
                # --- User-defined build options ---
                "-DBIT7Z_AUTO_FORMAT=ON",
                "-DBIT7Z_STATIC_RUNTIME=ON"
            )
            
            $commandString = "cmake " + ($cmakeConfigArgs -join " ")
            Write-Host "Command: $commandString"
            
            # Invoke the command
            & cmake $cmakeConfigArgs

            if ($LASTEXITCODE -ne 0) {
                Write-Warning "CMake configuration FAILED for $platform / $config."
                Write-Warning "Skipping build for this combination."
                continue # Skip to the next configuration
            }

            # --- 4. Build Step ---
            Write-Host "Running CMake Build..."
            $cmakeBuildArgs = @(
                "--build", $buildDir,
                "--config", $config
            )
            
            $commandString = "cmake " + ($cmakeBuildArgs -join " ")
            Write-Host "Command: $commandString"

            # Invoke the command
            & cmake $cmakeBuildArgs

            if ($LASTEXITCODE -ne 0) {
                Write-Warning "CMake build FAILED for $platform / $config."
            } else {
                Write-Host "Successfully built $platform / $config" -ForegroundColor Green
                Write-Host "Build artifacts are in the default bit7z library path (e.g., .../third-party/bit7z/lib/)"
            }
        }
    }
}
catch {
    Write-Error "An unexpected error occurred:"
    Write-Error $_.Exception.Message
    Write-Error "Script execution halted."
}

Write-Host "--------------------------------------------------"
Write-Host "Build script finished." -ForegroundColor Cyan