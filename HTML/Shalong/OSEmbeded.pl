#�����˷��� AKA OS&Embededɳ�� ֪ͨ����
use Mail::Sender;

#Email ���ܳ�ʼ�� �� ����֪ͨ����
$sender = new Mail::Sender
              {smtp => '166.111.26.54', from => 'aka@aka.org.cn'};

open (TONGZHI, "tongzhi.txt") || die ("Could not open tongzhi file\n");
@tongzhi = <TONGZHI>;
$subject = shift(@tongzhi);

#���͸� ɳ�����г�Ա
open (XCLIST, "OSEmbeded.txt") || die ("Could not open OSEmbeded.txt\n");
@xclist = <XCLIST>;

$prologue = "hi, dear friend:\n\n    ���ǰ��� OSEmbeded ɳ����֪ͨ����ӭ������Լǰ����\n    ���� egroups ���𻵣���������ֻ���ֹ�����ҷ��š�\n\n    �������ɳ���������������������� xuas\@263.net ��ϵ\n    ����ϣ���˳����б���� freeAKA\@263.net ���ţ�ע��<�˳� OS ɳ��>��лл��\n    �����������ϣ��������б�Ҳ��� freeAKA\@263.net ���ţ�ע��<���� OS ɳ��>��лл��\n\nAKA\n===============================\n";
unshift(@tongzhi, $prologue);

$signature = "\n\n====================================\n���ɡ�Э�������� �� Ϊ������\n��ӭ���ʣ�http://www.aka.org.cn  http://www.oshack.net\n�����Դ�ѩɽ�Ĵ��㰢����\n";
push(@tongzhi, $signature);

$count = 1;
while ($count <= @xclist) {
  $email = $xclist[$count-1];
  chomp($email);
  printf (".");  if ($count % 10 == 0) { printf ("$count"); }
  $count++;

  $sender->Open({to => $email, subject => $subject});
  $sender->Send(@tongzhi);
  $sender->Close;

  sleep(2);
}
close(XCLIST);
printf ("�����ʼ���ַ�� Finished!\n");

#����
close(TONGZHI);
printf ("����֪ͨ��������������     �� Enter ������\n");
$var = <STDIN>;