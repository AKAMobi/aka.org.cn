#!/usr/bin/perl

$password="aka"; #����Ϊaka

printf "������������?  ";
$name=<STDIN>;
chop($name);

if ($name eq "aka"){
  printf "��ӭ AKAer ���뱾ϵͳ!\n���������룺";
  $guess=<STDIN>;
  chop($guess);
 
  while ($guess ne $password){
    printf "����������������룺";
    $guess=<STDIN>;
    chop($guess);
  }
}
