param([ValidateSet('prepare','pre','post')][string]$Action='pre',
    [Parameter(Mandatory=$true)][string]$Project,[ValidateSet('boot','app','transfer')][string]$Kind)
$ErrorActionPreference='Stop'
$Project=[IO.Path]::GetFullPath((Resolve-Path -LiteralPath $Project).Path).TrimEnd('\','.')
$boot=Split-Path -Parent $PSScriptRoot
. (Join-Path $PSScriptRoot 'app_config.ps1')
. (Join-Path $PSScriptRoot 'evm_prepare.ps1')
$name=Split-Path -Leaf $Project
$fd=$name -match 'CAN_FD'
if(!$Kind){$Kind=if($name.EndsWith('_app')){'app'}else{'boot'}}
$isApp=$Kind -eq 'app'
$isTransfer=$Kind -eq 'transfer'
$utf8=New-Object Text.UTF8Encoding($false)
function Write-Generated([string]$Path,[string]$Text){
    if(!(Test-Path -LiteralPath $Path) -or [IO.File]::ReadAllText($Path) -cne $Text){[IO.File]::WriteAllText($Path,$Text,$utf8)}
}
function Read-Irq {
    $body=Get-AppConfigBody (Join-Path $Project 'app_irq_handlers.h') 'APP_IRQ_HANDLERS_H'
    $body=[regex]::Replace($body,'\\\r?\n',' ')
    $m=[regex]::Match($body,'\A\s*#define\s+APP_IRQ_HANDLERS\s*\(\s*X\s*\)(.*?)\s*\z','Singleline')
    if(!$m.Success){throw 'Expected APP_IRQ_HANDLERS(X).'}
    $rest=$m.Groups[1].Value;$mapping=@{}
    while($rest.Trim()){
        $m=[regex]::Match($rest,'\A\s*X\s*\(\s*([^,()]+)\s*,\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)')
        if(!$m.Success){throw 'Invalid IRQ entry; use X(EIINT_channel, global_handler).'}
        $ch=ConvertFrom-AppUnsignedLiteral $m.Groups[1].Value
        if($ch -gt 357 -or $mapping.ContainsKey([int]$ch)){throw 'Invalid/duplicate RH850/F1KM-S1 EIINT channel.'}
        $mapping[[int]$ch]=$m.Groups[2].Value;$rest=$rest.Substring($m.Length)
    }
    if($mapping[23] -cne 'Device_CanRxIsr' -or $mapping[84] -cne 'r_Config_OSTM0_interrupt'){throw 'CAN RX23 and OSTM84 are required by this App runtime.'}
    return $mapping
}
function Map-Symbol([string]$Symbol){
    $m=[regex]::Match($script:map,'(?m)^  _'+[regex]::Escape($Symbol)+'\r?\n\s+([0-9a-fA-F]{8})\s')
    if(!$m.Success){throw "Missing linked symbol $Symbol"}
    return [Convert]::ToUInt32($m.Groups[1].Value,16)
}
function Map-Section([string]$Symbol){
    $m=[regex]::Match($script:map,'(?m)^\s+'+[regex]::Escape($Symbol)+'\r?\n\s+([0-9a-fA-F]{8})\b')
    if(!$m.Success){throw "Missing linker section symbol $Symbol"}
    return [Convert]::ToUInt32($m.Groups[1].Value,16)
}
function Image-Define([string]$Key){
    $header=[IO.File]::ReadAllText((Join-Path $boot 'image_metadata_config.h'))
    $m=[regex]::Match($header,'(?m)^#define\s+'+[regex]::Escape($Key)+'\s+(\([^\r\n)]+\))')
    if(!$m.Success){throw "Missing literal image contract $Key"}
    return ConvertFrom-AppUnsignedLiteral $m.Groups[1].Value
}
function Read-Mot([string]$Path){
    $mem=New-Object 'Collections.Generic.Dictionary[uint32,byte]'
    $flashLimit=Image-Define IMAGE_CODE_FLASH_SIZE
    foreach($line in Get-Content -LiteralPath $Path){
        if($line -notmatch '^S([0-9])([0-9A-Fa-f]+)$'){throw 'Invalid S-record.'}
        $type=[int]$Matches[1];$hex=$Matches[2];$sum=0;$bytes=New-Object byte[] ($hex.Length/2)
        for($i=0;$i -lt $bytes.Length;$i++){$bytes[$i]=[Convert]::ToByte($hex.Substring($i*2,2),16);$sum+=$bytes[$i]}
        if($hex.Length%2 -or $bytes[0] -ne $bytes.Length-1 -or ($sum -band 255) -ne 255){throw 'Invalid S-record count/checksum.'}
        if($type -notin @(1,2,3)){continue}
        [uint32]$at=0
        for($i=1;$i -le $type+1;$i++){$at=($at -shl 8) -bor $bytes[$i]}
        for($i=$type+2;$i -lt $bytes.Length-1;$i++){
            if($at -ge $flashLimit -or $mem.ContainsKey($at)){throw 'Overlapping/out-of-flash MOT.'}
            $mem.Add($at,$bytes[$i]);$at++
        }
    }
    return ,$mem
}
function Mem32([uint32]$At){
    $v=New-Object byte[] 4
    for($i=0;$i -lt 4;$i++){if(!$script:mem.ContainsKey([uint32]($At+$i))){throw "Missing linked byte $At"};$v[$i]=$script:mem[[uint32]($At+$i)]}
    return [BitConverter]::ToUInt32($v,0)
}
function Write-Hex([string]$Path,[byte[]]$Data,[uint32]$Base){
    $lines=New-Object 'Collections.Generic.List[string]';$upper=-1
    for($off=0;$off -lt $Data.Length;$off+=16){
        $at=$Base+$off;$hi=$at -shr 16
        if($upper -ne $hi){$upper=$hi;$sum=2+4+($hi -shr 8)+($hi -band 255);$lines.Add((':02000004{0:X4}{1:X2}' -f $hi,((256-($sum -band 255)) -band 255)))}
        $count=[Math]::Min(16,$Data.Length-$off);$low=$at -band 65535;$sum=$count+($low -shr 8)+($low -band 255)
        $text=':{0:X2}{1:X4}00' -f $count,$low
        for($b=0;$b -lt $count;$b++){$sum+=$Data[$off+$b];$text+=$Data[$off+$b].ToString('X2')}
        $lines.Add($text+(((256-($sum -band 255)) -band 255).ToString('X2')))
    }
    $lines.Add(':00000001FF');[IO.File]::WriteAllLines($Path,$lines,[Text.Encoding]::ASCII)
}
if($Action -in @('prepare','pre')){
    if(!$isApp){Update-EvmClock $Project}
    $gen=Join-Path $Project 'generated_projection';New-Item -ItemType Directory -Path $gen -Force | Out-Null
    $mapping=if($isApp){Read-Irq}else{@{23='Device_CanRxIsr';84='r_Config_OSTM0_interrupt'}}
    $lines=New-Object 'Collections.Generic.List[string]'
    $lines.Add('/* Generated from local C headers; EIINT channel indexes, not byte addresses. */')
    $lines.Add('#include "r_cg_macrodriver.h"')
    foreach($s in @($mapping.Values | Sort-Object -Unique)){$lines.Add('extern void '+$s+'(void);')}
    $lines.Add('#pragma interrupt Device_UnhandledIsr(enable=false, fpu=true, callt=false)')
    foreach($line in @('void Device_UnhandledIsr(void)','{','    DI();','    for (;;)','    {','        NOP();','    }','}')){$lines.Add($line)}
    $lines.Add('#pragma section const "EIINTTBL"')
    $lines.Add('void (* const INT_Vectors[512])(void) = {')
    for($i=0;$i -lt 512;$i++){$s=if($mapping.ContainsKey($i)){$mapping[$i]}else{'Device_UnhandledIsr'};$lines.Add('    '+$s+', /* EIINT '+$i+' */')}
    $lines.Add('};');Write-Generated (Join-Path $gen 'device_vectors.c') ($lines -join "`r`n")
    $timer=[IO.File]::ReadAllText((Join-Path $Project 'src\smc_gen\Config_OSTM0\Config_OSTM0_user.c'))
    Write-Generated (Join-Path $gen 'device_timer.c') ($timer -replace ',\s*channel=84','')
    if($isApp){
        $identity=Read-AppImageConfig $Project 0x8501
        $header="/* Generated from app_image_config.h; do not edit. */`r`n#ifndef APP_IDENTITY_H`r`n#define APP_IDENTITY_H`r`n"
        foreach($key in @('Version','Build','VersionControl')){
            $macro=@{Version='VERSION';Build='BUILD';VersionControl='VERSION_CONTROL'}[$key]
            $header+='#define DEVICE_APP_'+$macro+(' (0x{0:X8}UL)' -f $identity[$key])+"`r`n"
        }
        Write-Generated (Join-Path $gen 'app_identity.h') ($header+"#endif`r`n")
        $system=[IO.File]::ReadAllText((Join-Path $Project 'src\smc_gen\general\r_cg_systeminit.c'))
        # App inherits the Boot clock; all other SC peripheral initialization is retained.
        $system=$system -replace 'R_CGC_Create\(\);','/* Clock is inherited from Boot and checked before initialization. */'
        Write-Generated (Join-Path $gen 'app_system.c') $system
    }
    if($Action -eq 'pre'){return}
    # Source/linker/SC settings remain owned by this local project.
    return
}

$out=Join-Path $Project 'DefaultBuild'
$map=[IO.File]::ReadAllText((Join-Path $out ($name+'.map')))
$mem=Read-Mot (Join-Path $out ($name+'.mot'))
$low=if($isApp){0x10000}else{0};$code=if($isApp){0x11000}else{0x1000};$table=if($isApp){0x10400}else{0x200}
$high=[uint32]($mem.Keys | Measure-Object -Maximum).Maximum
if(($mem.Keys | Measure-Object -Minimum).Minimum -lt $low -or (!$isApp -and $high -ge 0xFE00)){throw 'Image exceeds its partition/spare guard.'}
$mapping=if($isApp){Read-Irq}else{@{23='Device_CanRxIsr';84='r_Config_OSTM0_interrupt'}}
$rows=New-Object Text.StringBuilder
for($i=0;$i -lt 512;$i++){
    $handler=if($mapping.ContainsKey($i)){$mapping[$i]}else{'Device_UnhandledIsr'}
    $value=Mem32 ($table+$i*4)
    if($value -ne (Map-Symbol $handler) -or $value -lt $code -or !$mem.ContainsKey([uint32]$value)){throw "Stale linked IRQ table at channel $i"}
    [void]$rows.AppendLine(('<tr><td>{0}</td><td>0x{1:X8}</td><td>{2}</td></tr>' -f $i,$value,$handler))
}
# RAM allocations must not collide with the E2 monitor, reader, FCL or cookie.
$stack=Map-Symbol 'stacktop'
if($isTransfer){
    if($stack -gt [Convert]::ToUInt32('FEDF4800',16)){throw 'Transfer RAM collides with debugger area.'}
    Write-Host ('PASS: FD Transfer MOT/512 IRQ destinations; ROM high 0x{0:X8}. Hardware NOT RUN.' -f $high)
    return
}
if($stack -gt [Convert]::ToUInt32('FEDF4800',16) -or (Map-Symbol 'g_read_ram') -ne [Convert]::ToUInt32('FEDF7000',16)){throw 'RAM layout collides with reserved areas.'}
$ramReader=Map-Symbol 'Device_ReadRawRam'
$readerStart=Map-Section '__sRAM_READ.text';$readerEnd=Map-Section '__eRAM_READ.text'
if($ramReader -ne $readerStart -or $readerEnd -le $readerStart -or $readerEnd-$readerStart -gt 4096){throw 'RAM reader copy extent/function start mismatch.'}
if(!$isApp){
    $begin=Map-Section '__sR_FCL_CODE_USRINT.text';$usr=Map-Section '__sR_FCL_CODE_USR.text'
    $ram=Map-Section '__sR_FCL_CODE_RAM.text';$romram=Map-Section '__sR_FCL_CODE_ROMRAM.text'
    $ex=Map-Section '__sR_FCL_CODE_RAM_EX_PROT.text';$end=Map-Section '__eR_FCL_CODE_RAM_EX_PROT.text'
    if($usr -lt $begin -or $ram -lt $usr -or $romram -lt $ram -or $ex -lt $romram -or $end -le $ex -or
       $romram-$begin+$end-$ex -gt 0x7000 -or (Map-Symbol 'g_fcl_ram') -ne [Convert]::ToUInt32('FEDF8000',16)){
        throw 'FCL internal-handler copy order/extent exceeds its reserved RAM.'
    }
}
$asm=[IO.File]::ReadAllText((Join-Path $out 'device_read.asm'))
$body=[regex]::Match($asm,'(?s)_Device_ReadRawRam:(.*?)(?=\s*\.section|\z)').Groups[1].Value
if(!$body -or $body -match '\b(jarl|jarl32|callt)\b'){throw 'RAM reader contains an external call or no assembly evidence.'}
if(!$isApp){
    $bytes=New-Object byte[] 0x10000;for($i=0;$i -lt $bytes.Length;$i++){$bytes[$i]=255}
    foreach($at in $mem.Keys){$bytes[$at]=$mem[$at]}
    Write-Hex (Join-Path $out 'boot0000_FFFF.hex') $bytes 0
}else{
    $id=Read-AppImageConfig $Project 0x8501
    $length=([int]($high-$low+1)+3) -band -4;$length+=4
    if($length -gt (Image-Define IMAGE_APPLICATION_CAPACITY)){throw 'App exceeds capacity.'}
    $bytes=New-Object byte[] $length;for($i=0;$i -lt $length;$i++){$bytes[$i]=255}
    foreach($at in $mem.Keys){$bytes[$at-$low]=$mem[$at]}
    $expected=[uint32[]]@((Image-Define IMAGE_MANIFEST_MAGIC),(Image-Define IMAGE_MANIFEST_FORMAT_WORD),0x01050001,
        ((Image-Define IMAGE_TARGET_ID) -bor ($(if($fd){2}else{1}) -shl 16)),$id.Version,$id.Build,[uint32]::MaxValue,
        (Image-Define IMAGE_APPLICATION_START),(Image-Define IMAGE_APPLICATION_CAPACITY),(Image-Define IMAGE_CODE_FLASH_SIZE),
        (Image-Define IMAGE_RESERVED_SIZE),(Image-Define IMAGE_ERASE_BLOCK_SIZE),4,$(if($fd){2}else{0}),0x700,0x701,
        (Image-Define IMAGE_MCU_PART_0),(Image-Define IMAGE_MCU_PART_1),(Image-Define IMAGE_MCU_PART_2),(Image-Define IMAGE_MCU_PART_3),
        (Image-Define IMAGE_MCU_FAMILY_0),(Image-Define IMAGE_MCU_FAMILY_1),(Image-Define IMAGE_MCU_FAMILY_2),
        (Image-Define IMAGE_MINIMUM_IMAGE_SIZE),0x31445055,(Image-Define IMAGE_POLICY_WORD),(Image-Define IMAGE_IRQ_ABI),
        (Image-Define IMAGE_BOOT_VERSION),0,$id.VersionControl,0,0)
    for($i=0;$i -lt 64;$i++){
        $want=if($i -lt 32){$expected[$i]}else{[uint32]::MaxValue}
        if((Mem32 (0x10100+$i*4)) -ne $want){throw "Stale compiled metadata word $i"}
    }
    [Array]::Copy([BitConverter]::GetBytes([uint32]$length),0,$bytes,0x118,4)
    # Compile the CRC loop once in the local .NET runtime, no extra installation.
    if(!('Rh850BuildCrc' -as [type])){Add-Type 'public static class Rh850BuildCrc { public static uint Run(byte[] d,int n) { uint c=0xffffffff; for(int i=0;i<n;i++){c^=d[i];for(int b=0;b<8;b++)c=(c&1)!=0?(c>>1)^0xedb88320:c>>1;}return c^0xffffffff;} }'}
    $crc=[Rh850BuildCrc]::Run($bytes,$length-4);[Array]::Copy([BitConverter]::GetBytes($crc),0,$bytes,$length-4,4)
    $bin=Join-Path $out ($name+'_update.bin');[IO.File]::WriteAllBytes($bin,$bytes)
    Write-Hex (Join-Path $out 'flash_app_variable_CRC.hex') $bytes $low
    $hash=(Get-FileHash -LiteralPath $bin -Algorithm SHA256).Hash
    $metaRows=New-Object Text.StringBuilder
    for($i=0;$i -lt 256;$i++){[void]$metaRows.AppendLine(('<tr><td>0x{0:X8}</td><td>0x{1:X2}</td></tr>' -f (0x10100+$i),$bytes[0x100+$i]))}
    $html='<!doctype html><html lang="en"><meta charset="utf-8"><title>App build report</title><style>body{font:16px system-ui;margin:2em}table{border-collapse:collapse}td,th{border:1px solid #aaa;padding:.3em}</style><h1>'+[Net.WebUtility]::HtmlEncode($name)+'</h1><p>Offline compiled image report. Hardware tests are not implied.</p><p>BIN: '+$length+' bytes; CRC32: '+$crc.ToString('X8')+'; SHA256: '+$hash+'</p><h2>Metadata bytes</h2><table><tr><th>Address</th><th>Byte</th></tr>'+$metaRows+'</table><h2>EIINT table</h2><table><tr><th>Channel</th><th>Destination</th><th>Handler</th></tr>'+$rows+'</table></html>'
    Write-Generated (Join-Path $out 'app_build_report.html') $html
}
Write-Host ('PASS: actual MOT/metadata/512 IRQ destinations/RAM reader; ROM high 0x{0:X8}. Hardware NOT RUN.' -f $high)

# APP_TIMING_BINS_BEGIN
if($isApp) {
    $LASTEXITCODE=0
    & (Join-Path $Project 'generate_test_bins.ps1') -ProjectRoot $Project -Automatic
    if($LASTEXITCODE -ne 0){throw 'App timing BIN generation failed.'}
}
# APP_TIMING_BINS_END
