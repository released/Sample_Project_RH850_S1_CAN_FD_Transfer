# Golden CAN-device authoring contract. Windows PowerShell 5.1 / .NET only.
# Identical local copies are supplied with each independent Boot repository.
function Get-AppConfigBody([string]$Path, [string]$Guard) {
    $text=[IO.File]::ReadAllText($Path).Replace("`r`n","`n")
    $text=[regex]::Replace($text,'(?s)/\*.*?\*/',' ')
    if($text.Contains('/*') -or $text.Contains('*/') -or $text.Contains('//')) {
        throw "Invalid C90 configuration comment: $Path"
    }
    $pattern='\A\s*#ifndef\s+'+[regex]::Escape($Guard)+'\s*\n\s*#define\s+'+
        [regex]::Escape($Guard)+'\s*\n(?<body>.*?)\n\s*#endif\s*\z'
    $match=[regex]::Match($text,$pattern,[Text.RegularExpressions.RegexOptions]::Singleline)
    if(!$match.Success) { throw "Expected guarded configuration header $Guard : $Path" }
    return $match.Groups['body'].Value
}
function ConvertFrom-AppUnsignedLiteral([string]$Literal) {
    $value=$Literal.Trim()
    if($value.StartsWith('(') -and $value.EndsWith(')')) { $value=$value.Substring(1,$value.Length-2).Trim() }
    $match=[regex]::Match($value,'\A(0[xX][0-9A-Fa-f]+|0|[1-9][0-9]*)(?:[uU][lL]?|[lL][uU]?)?\z')
    if(!$match.Success) { throw "Expected unsigned C integer literal: $Literal" }
    $digits=$match.Groups[1].Value
    $number=if($digits -match '^0[xX]') {[Convert]::ToUInt64($digits.Substring(2),16)}else{[Convert]::ToUInt64($digits,10)}
    if($number -gt [uint32]::MaxValue) { throw "C configuration value exceeds uint32: $Literal" }
    return [uint32]$number
}
function Read-AppImageConfig([string]$Project, [uint32]$ExpectedTargetId) {
    $body=Get-AppConfigBody (Join-Path $Project 'app_image_config.h') 'APP_IMAGE_CONFIG_H'
    $values=@{}
    foreach($line in $body.Split("`n")) {
        if(!$line.Trim()) { continue }
        $match=[regex]::Match($line,'^\s*#define\s+(APP_IMAGE_[A-Z_]+)\s+(.+?)\s*$')
        if(!$match.Success -or $values.ContainsKey($match.Groups[1].Value)) {
            throw 'Invalid/duplicate app_image_config.h definition.'
        }
        $values[$match.Groups[1].Value]=ConvertFrom-AppUnsignedLiteral $match.Groups[2].Value
    }
    $expected='APP_IMAGE_BUILD,APP_IMAGE_ROLE,APP_IMAGE_TARGET_ID,APP_IMAGE_UPDATE_TRANSPORTS,APP_IMAGE_VERSION,APP_IMAGE_VERSION_CONTROL'
    if((@($values.Keys | Sort-Object) -join ',') -cne $expected) {
        throw 'app_image_config.h requires version/build/target ID/role/update transports/version control.'
    }
    if($values.APP_IMAGE_VERSION -gt 0xFFFFFF -or $values.APP_IMAGE_BUILD -eq [uint32]::MaxValue -or
        $values.APP_IMAGE_VERSION_CONTROL -gt 1) {
        throw 'App version must be 0x00MMmmpp, build below 0xFFFFFFFF, version control 0 or 1.'
    }
    if($values.APP_IMAGE_TARGET_ID -ne $ExpectedTargetId -or $values.APP_IMAGE_ROLE -ne 1 -or
        $values.APP_IMAGE_UPDATE_TRANSPORTS -ne 1) {
        throw 'App target/role/transport must match the CAN DEVICE_APP Boot contract.'
    }
    return @{Version=$values.APP_IMAGE_VERSION;Build=$values.APP_IMAGE_BUILD;Target=$values.APP_IMAGE_TARGET_ID;VersionControl=$values.APP_IMAGE_VERSION_CONTROL}
}
function Read-AppIrqConfig([string]$Project) {
    $body=Get-AppConfigBody (Join-Path $Project 'app_irq_handlers.h') 'APP_IRQ_HANDLERS_H'
    $body=[regex]::Replace($body,'\\\n',' ')
    $macro=[regex]::Match($body,'\A\s*#define\s+APP_IRQ_HANDLERS\s*\(\s*X\s*\)(?<entries>.*?)\s*\z',
        [Text.RegularExpressions.RegexOptions]::Singleline)
    if(!$macro.Success) { throw 'Expected APP_IRQ_HANDLERS(X) in app_irq_handlers.h.' }
    $remaining=$macro.Groups['entries'].Value
    $mapping=@{}
    while($remaining.Trim()) {
        $entry=[regex]::Match($remaining,'\A\s*X\s*\(\s*([^,()]+)\s*,\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)')
        if(!$entry.Success) { throw 'Invalid APP_IRQ_HANDLERS entry; use X(vector, handler).' }
        $vector=ConvertFrom-AppUnsignedLiteral $entry.Groups[1].Value
        if($vector -lt 4 -or $vector -gt 124 -or ($vector -band 1) -or $mapping.ContainsKey([int]$vector)) {
            throw 'Invalid/duplicate normal IRQ vector in app_irq_handlers.h.'
        }
        $mapping[[int]$vector]=$entry.Groups[2].Value
        $remaining=$remaining.Substring($entry.Length)
    }
    return $mapping
}
