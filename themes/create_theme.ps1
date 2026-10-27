param(
    [string]$ThemeFile,
    [string]$OutputDirectory
)

$ErrorActionPreference = 'Stop'
$DockerImage = 'devkitpro/devkitarm:latest'

if ([string]::IsNullOrWhiteSpace($ThemeFile) -or [string]::IsNullOrWhiteSpace($OutputDirectory)) {
    Write-Host 'Usage: .\create-theme.ps1 <theme file> <output directory>'
    exit 1
}

$workDir = $null

try {
    if (-not (Test-Path -LiteralPath $ThemeFile -PathType Leaf)) {
        throw "Theme file not found: $ThemeFile"
    }

    $inputPath = (Resolve-Path -LiteralPath $ThemeFile).ProviderPath
    $inputDir = [IO.Path]::GetDirectoryName($inputPath)
    $themeName = [IO.Path]::GetFileNameWithoutExtension($inputPath)

    New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null
    $outputDir = (Resolve-Path -LiteralPath $OutputDirectory).ProviderPath

    if ($inputDir.TrimEnd('\') -ieq $outputDir.TrimEnd('\')) {
        throw 'The output directory must differ from the theme source directory.'
    }

    $lines = [IO.File]::ReadAllLines($inputPath)
    $bitmaps = @()
    $sounds = @()

    foreach ($line in $lines) {
        if ($line -match '^\s*\w*Bitmap\s+(\S+)') {
            if ($bitmaps -notcontains $Matches[1]) { $bitmaps += $Matches[1] }
        }
        if ($line -match '^\s*Sound\w*\s+(\S+)') {
            if ($sounds -notcontains $Matches[1]) { $sounds += $Matches[1] }
        }
    }

    Copy-Item -LiteralPath $inputPath -Destination (Join-Path $outputDir "$themeName.ibt") -Force

    if ($bitmaps.Count -gt 0) {
        if (-not (Get-Command docker -ErrorAction SilentlyContinue)) {
            throw 'Docker not found. Install Docker Desktop and start it.'
        }

        # Temporary build directory: never delete the source PNG files.
        $workDir = Join-Path $outputDir ('.theme-build-' + [guid]::NewGuid().ToString('N'))
        New-Item -ItemType Directory -Path $workDir | Out-Null
        $t3sLines = @('--atlas -f rgba8888 -z auto')

        foreach ($bitmap in $bitmaps) {
            $png = Join-Path $inputDir "$bitmap.png"
            if (-not (Test-Path -LiteralPath $png -PathType Leaf)) {
                throw "Cannot find bitmap: $png"
            }
            Copy-Item -LiteralPath $png -Destination (Join-Path $workDir "$bitmap.png")
            $t3sLines += "$bitmap.png"
        }

        $utf8 = [System.Text.UTF8Encoding]::new($false)
        [IO.File]::WriteAllLines((Join-Path $workDir "$themeName.t3s"), [string[]]$t3sLines, $utf8)

        & docker run --rm -v "${workDir}:/work" -w /work $DockerImage `
            tex3ds -i "$themeName.t3s" -H "$themeName.h" -o "$themeName.t3x"
        if ($LASTEXITCODE -ne 0) { throw "tex3ds failed (exit code $LASTEXITCODE)." }

        Copy-Item -LiteralPath (Join-Path $workDir "$themeName.h") -Destination $outputDir -Force
        Copy-Item -LiteralPath (Join-Path $workDir "$themeName.t3x") -Destination $outputDir -Force
    }

    if ($sounds.Count -gt 0) {
        if (-not (Get-Command ffmpeg -ErrorAction SilentlyContinue)) {
            Write-Warning 'ffmpeg not found. Removing sound directives from the output theme.'
            $filteredLines = @($lines | Where-Object { $_ -notmatch '^\s*Sound\w*\s+' })
            $utf8 = [System.Text.UTF8Encoding]::new($false)
            [IO.File]::WriteAllLines((Join-Path $outputDir "$themeName.ibt"), [string[]]$filteredLines, $utf8)
        } else {
            foreach ($sound in $sounds) {
                $wav = Join-Path $inputDir "$sound.wav"
                $mp3 = Join-Path $inputDir "$sound.mp3"

                if (Test-Path -LiteralPath $wav -PathType Leaf) {
                    $soundInput = $wav
                } elseif (Test-Path -LiteralPath $mp3 -PathType Leaf) {
                    $soundInput = $mp3
                } else {
                    throw "Cannot find sound: $wav or $mp3"
                }

                & ffmpeg -hide_banner -loglevel error -nostats -y -i $soundInput `
                    -ac 1 -ar 22050 -c:a pcm_s16le -f s16le (Join-Path $outputDir "$sound.raw")
                if ($LASTEXITCODE -ne 0) { throw "ffmpeg failed for $sound (exit code $LASTEXITCODE)." }
            }
        }
    }

    Write-Host "Theme created. Copy the contents of $outputDir to sdmc:/icebreaker"
}
catch {
    Write-Error $_.Exception.Message
    exit 1
}
finally {
    if ($workDir -and (Test-Path -LiteralPath $workDir)) {
        Remove-Item -LiteralPath $workDir -Recurse -Force -ErrorAction SilentlyContinue
    }
}
