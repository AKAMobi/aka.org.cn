#�����˷��� AKA ֪ͨ����
use Mail::Sender;

#Email ���ܳ�ʼ�� �� ����֪ͨ����
$sender = new Mail::Sender
              {smtp => '166.111.26.54', from => 'aka@aka.org.cn'};

open (TONGZHI, "tongzhi.txt") || die ("Could not open tongzhi file\n");
@tongzhi = <TONGZHI>;
$subject = shift(@tongzhi);

$prologue = "\n    �������֪ͨ����ӭ������Լǰ����\n    ����ϣ���˳����б���� freeAKA\@263.net ���ţ�ע��<�˳� AKA �֪ͨ>��лл��\n    �����������ϣ��������б�Ҳ��� freeAKA\@263.net ���ţ�ע��<���� AKA �֪ͨ>��лл��\n\nAKA\n\n===============================\n";
unshift(@tongzhi, $prologue);
$signature = "\n\n====================================\n���ɡ�Э�������� �� Ϊ������\n��ӭ���ʣ�http://www.aka.org.cn \n�����Դ�ѩɽ�Ĵ��㰢����\n";
push(@tongzhi, $signature);

#���͸������ʼ��б�
open (BMLIST, "�������ʼ��б�.txt") || die ("Could not open �������ʼ��б�\n");
@bmlist = <BMLIST>;

$count = 1;
while ($count <= @bmlist) {
  $email = $bmlist[$count-1];
  chomp($email);
  printf (".");  if ($count % 10 == 0) { printf ("$count"); }
  $count++;

  my @temp = @tongzhi;
  my $to = "To: ".$email."\n\n";
  unshift(@temp, $to);
  $sender->Open({to => $email, subject => $subject});
  $sender->Send(@temp);
  $sender->Close;

  sleep(2);
}
close(BMLIST);
printf("�������ʼ��б� Finished!\n\n");

#���͸� ������ַ�б�
open (XCLIST, "�����ʼ���ַ��.txt") || die ("Could not open �����ʼ���ַ��\n");
@xclist = <XCLIST>;

$prologue = "hi, dear friend:\n\n    ���ǰ����֪ͨ���������Ȥ����ӭ�����μӡ�\n    ���п��ܣ����æ�Ѹ���Ϣת���������ϵ�������������飬лл��\n\n";
unshift(@tongzhi, $prologue);

$count = 1;
while ($count <= @xclist) {
  $email = $xclist[$count-1];
  chomp($email);
  printf (".");  if ($count % 10 == 0) { printf ("$count"); }
  $count++;

  my @temp = @tongzhi;
  my $to = "To: ".$email."\n\n";
  unshift(@temp, $to);
  $sender->Open({to => $email, subject => $subject});
  $sender->Send(@temp);
  $sender->Close;

  sleep(2);
}
close(XCLIST);
printf ("�����ʼ���ַ�� Finished!\n\n");

#����
close(TONGZHI);
printf ("����֪ͨ��������������     �� Enter ������\n");
$var = <STDIN>;