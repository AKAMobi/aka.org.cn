#! /usr/bin/perl -w

use HTML::Parser ;
use CGI;
use Fcntl ':flock';

$q = new CGI;
print $q->header;

@names = $q->param;

if( $q->param('select') eq '�ύ' ){
	open( FD, ">>Student.xml" ) or die "error open xml file";
	flock( FD, LOCK_EX );
	print FD "<Student>\n";
	foreach $name ( @names ){
		next if ( $name eq 'select' );
		print FD "\t<$name>" . $q->param($name) . "</$name>\n";
	}
	print FD "</Student>\n";
	flock( FD, LOCK_UN );
	close( FD );
}

print "<h1>��л���Ĳ��룬лл��</h1><br>";
print "���<a href=\"http://www.aka.org.cn\">����</a>������ AKA ��ҳ";


$p = HTML::Parser->new ;

@Students = ();
%Vote = ();

$p->parse_file ("/home/akaWWW/cgi-bin/Student.xml") ;


print "<hr><br><center>���µ�����<center><br>";

print "<table width=750 border=0><tr bgcolor='999999'><td width=150>����ǰ�Ľ����Ƿ�����</td><td width=150>���������Ȥ�Ľ���</td><td width=150>���Դ�ѧ��˾</td><td width=150>����ֲ�����30��Ϊ�磩</td><td width=150>����������ü</td></tr></table>";
print "<table border=0><tr>";
foreach $item ( keys %Vote ){
	print "<td><table width=150 border=0>";
	foreach ( keys %{$Vote{$item}} ){
		print "<tr><td>$_</td><td>&nbsp&nbsp" . $Vote{$item}{$_} . "&nbsp&nbsp</td></tr>";
	}
	print "</table></td>";
}
print "</tr></table>\n";
		
print "</body></html>";
exit;


sub HTML::Parser::start{
	local ($self,$tag, $attr, $attrseq, $origtext) = @_ ;
	$self->{tag} = $tag;
}

sub HTML::Parser::text{
	local ($self,$text) = @_ ;
	if( ! $self->{tag} ){
		return;
	}
	if( ! $text ){
		return;
	}
	if( $text eq "�ύ" || $text eq '�����' ){
		return;
	}
	if ($self->{tag} eq "username"){
		$Student{$self->{tag}} = $text;
	}elsif ($self->{tag} eq "email"){
		$Student{$self->{tag}} = $text;
	}elsif ($self->{tag} eq "address"){
		$Student{$self->{tag}} = $text;
	}elsif( $self->{tag} ne "student" ){
		$Vote{$self->{tag}}{$text}++;
	}
}

sub HTML::Parser::end{
	local ($self,$tag, $origtext) = @_ ;
	$self->{tag} = 0;
	#print "tag: $tag end\n" ;
	if( $tag eq "student" ){
		return unless $Student{username};
		local %tmpStudent;
		%tmpStudent = %Student;
		%Student = ();
		push( @Students, \%tmpStudent );
	}
}
