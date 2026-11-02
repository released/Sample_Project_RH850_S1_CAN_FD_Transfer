# Project-local firmware sources. This helper never falls back to a sibling checkout.
function Get-DeviceProjectSource([string]$Root,[string]$Relative) {
    $rootPath=[IO.Path]::GetFullPath($Root).TrimEnd('\')
    if([IO.Path]::IsPathRooted($Relative) -or $Relative -match '(^|[\\/])\.\.([\\/]|$)') {
        throw 'Device source must be relative to the selected project.'
    }
    $direct=[IO.Path]::GetFullPath((Join-Path $rootPath $Relative))
    if(!$direct.StartsWith($rootPath+'\',[StringComparison]::OrdinalIgnoreCase)) {
        throw 'Device source escaped its project.'
    }
    if(Test-Path -LiteralPath $direct -PathType Leaf){return $direct}
    $name=[IO.Path]::GetFileName($Relative.Replace('/','\'))
    $matches=@()
    foreach($folder in @('hal\device','hal\image','hal\flash','hal\isp','hal\can')) {
        $candidate=Join-Path (Join-Path $rootPath $folder) $name
        if(Test-Path -LiteralPath $candidate -PathType Leaf){$matches+=$candidate}
    }
    if($matches.Count -ne 1){throw ('Missing or ambiguous project-local source: '+$Relative)}
    return $matches[0]
}

function Assert-DeviceLocalBindings([string]$Project) {
    $root=(Resolve-Path -LiteralPath $Project).Path.TrimEnd('\')
    $name=Split-Path -Leaf $root
    [xml]$xml=[IO.File]::ReadAllText((Join-Path $root ($name+'.mtpj')))
    foreach($node in $xml.SelectNodes('//Instance[Type="File"]')) {
        $relative=[string]$node.RelativePath
        if(!$relative){continue}
        if([IO.Path]::IsPathRooted($relative) -or $relative -match '(^|[\\/])\.\.([\\/]|$)') {
            throw ('Non-local project source: '+$relative)
        }
        $path=[IO.Path]::GetFullPath((Join-Path $root $relative))
        if(!$path.StartsWith($root+'\',[StringComparison]::OrdinalIgnoreCase)) {
            throw ('Project binding escaped its root: '+$relative)
        }
        if(!(Test-Path -LiteralPath $path -PathType Leaf)){throw ('Missing local project binding: '+$relative)}
    }
}
