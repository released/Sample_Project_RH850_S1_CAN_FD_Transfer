param([string]$Compiler='')
$ErrorActionPreference='Stop'
$root=$PSScriptRoot
$name=Split-Path -Leaf $root
$project=Join-Path $root ($name+'.mtpj')
if(!(Test-Path -LiteralPath $project -PathType Leaf)){throw 'Missing project-local CS+ project.'}
if(!$Compiler){$Compiler=Join-Path ${env:ProgramFiles(x86)} 'Renesas Electronics\CS+\CC\CubeSuite+.exe'}
if(!(Test-Path -LiteralPath $Compiler -PathType Leaf)){throw 'Installed CS+ is required; use -Compiler to select CubeSuite+.exe.'}
$logs=Join-Path $root 'tmp\build'
[IO.Directory]::CreateDirectory($logs)|Out-Null
$log=Join-Path $logs ($name+'_rebuild.log')
$process=Start-Process -FilePath $Compiler -ArgumentList ('/br "'+$project+'"') -RedirectStandardOutput $log -RedirectStandardError ($log+'.stderr') -WindowStyle Hidden -PassThru -Wait
$result=[IO.File]::ReadAllText($log)
$result -split '\r?\n'|Select-Object -Last 18|Write-Host
if($result -notmatch 'Build ended\(Error:0, Warning:\d+\)' -or $result -match 'Failed:[1-9]'){throw ('CS+ rebuild failed: '+$log)}
Write-Host 'PASS project-local target build and pre/post guards. Hardware NOT RUN.'
