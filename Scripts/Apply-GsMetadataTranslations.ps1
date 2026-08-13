[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$TargetRoot,[Parameter(Mandatory=$true)][string]$TargetName)
$ErrorActionPreference='Stop'
$SeedPath=Join-Path $TargetRoot "$TargetName.metadata.tsv"
if(-not(Test-Path -LiteralPath $SeedPath)){throw "Metadata translation seed not found: $SeedPath"}
function Escape-Po([string]$Value){$Value.Replace('\','\\').Replace('"','\"').Replace("`r",'').Replace("`n",'\n')}
$ByCulture=@{}
foreach($Line in Get-Content -LiteralPath $SeedPath -Encoding UTF8){
 if([string]::IsNullOrWhiteSpace($Line)-or $Line.StartsWith('#')){continue}
 $Parts=$Line.Split("`t",3); if($Parts.Count-ne 3){throw "Invalid metadata translation row: $Line"}
 if(-not $ByCulture.ContainsKey($Parts[0])){$ByCulture[$Parts[0]]=@{}}
 $ByCulture[$Parts[0]][(Escape-Po $Parts[1])]=(Escape-Po $Parts[2])
}
$Total=0
foreach($Culture in $ByCulture.Keys){
 $PoPath=Join-Path $TargetRoot "$Culture\$TargetName.po"; if(-not(Test-Path -LiteralPath $PoPath)){continue}
 $Lines=[System.Collections.Generic.List[string]]::new(); foreach($Line in Get-Content -LiteralPath $PoPath -Encoding UTF8){$Lines.Add($Line)}
 $Current=$null;$Changes=0
 for($I=0;$I-lt $Lines.Count;$I++){if($Lines[$I]-match '^msgid "(.*)"$'){$Current=$Matches[1];continue};if($null-ne $Current-and $Lines[$I]-match '^msgstr ".*"$'){if($ByCulture[$Culture].ContainsKey($Current)){$Lines[$I]='msgstr "'+$ByCulture[$Culture][$Current]+'"';$Changes++};$Current=$null}}
 [System.IO.File]::WriteAllLines($PoPath,$Lines,[System.Text.UTF8Encoding]::new($false));$Total+=$Changes;Write-Host "[GsLocalization] $Culture / $Changes"
}
Write-Host "[GsLocalization] metadata translations applied: $Total"
