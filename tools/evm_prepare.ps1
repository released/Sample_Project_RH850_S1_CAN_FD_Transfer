param([string]$ClockProject='')
# Owned EVM preparation. Generated SC sources remain untouched.
function Write-EvmDerived([string]$Path,[string]$Text) {
    if(!(Test-Path -LiteralPath $Path) -or [IO.File]::ReadAllText($Path) -cne $Text){
        [IO.File]::WriteAllText($Path,$Text,(New-Object Text.UTF8Encoding($false)))
    }
}
function Update-EvmClock([string]$Root) {
    $source=Join-Path $Root 'src\smc_gen\general\r_cg_cgc.c'
    $text=[IO.File]::ReadAllText($source)
    if($text -notmatch 'CLKCTL.PLL1C\s*=.*_CGC_PLL1_DIVISION_RATIO'){
        throw 'Expected SC PLL1 assignment for EVM clock projection.'
    }
    # 16MHz * (Mr+1=30) =480MHz VCO;existing dividers retain CPU120/CAN40.
    $text=$text.Replace('_CGC_MAINOSC_8MHZ','_CGC_MAINOSC_16MHZ')
    $text=$text.Replace('_CGC_PLL1_DIVISION_RATIO','(0x0000001DUL)')
    $text=$text.Replace('R7F701692','R7F701684')
    $directory=Join-Path $Root 'generated_projection'
    New-Item -ItemType Directory -Path $directory -Force | Out-Null
    Write-EvmDerived (Join-Path $directory 'evm_clock.c') $text
}
function Set-EvmProject([string]$Root,[xml]$Xml) {
    foreach($node in $Xml.SelectNodes('//*[not(*)]')){
        if($node.InnerText.Contains('R7F701692')){
            $node.InnerText=$node.InnerText.Replace('R7F701692','R7F701684')
        }
    }
    $name=Split-Path -Leaf $Root
    $sc=Join-Path $Root ($name+'.scfg')
    $text=[IO.File]::ReadAllText($sc)
    $text=$text.Replace('platform id="R7F701692"','platform id="R7F701684"')
    $text=$text.Replace('id="main_frequency" selection="main_frequency_8"',
        'id="main_frequency" selection="main_frequency_16"')
    Write-EvmDerived $sc $text
    $startup=Join-Path $Root 'boot.asm'
    $text=[IO.File]::ReadAllText($startup)
    # Initialize the96KiB local RAM and the retained32KiB range as the
    # project startup already requires;preserve its existing cookie exclusion.
    $text=$text.Replace('0xfedf0000','0xfede0000')
    Write-EvmDerived $startup $text
    Update-EvmClock $Root
}
if($ClockProject){Update-EvmClock ([IO.Path]::GetFullPath($ClockProject))}
