#! /usr/bin/perl -w

use CGI;

$query = new CGI;

@names = $query->param;

open( FD, ">>Student.xml" ) or die "error open xml file";
print FD "<Student>\n";
foreach $name ( @names ){
	print FD "\t<$name>" . $query->param($name) . "</$name>\n";
}
print FD "</Student>\n";


print $query->header;
print "<h1>��л���Ĳ��룬лл��</h1><br><br><br>";
print "���<a href=\"http://www.aka.org.cn\">����</a>������ AKA ��ҳ";

