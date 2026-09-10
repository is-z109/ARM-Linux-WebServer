#include"log.h"
#include"tool.h"
LOG* LOG::instance()
{
	static LOG unique;
	return &unique;
}
void LOG::writefunction()
{
	string buffer;
	file.open("log.txt", ios::app);
	while (block_queue->pop(buffer))
	{
		file.write(buffer.data(), buffer.size());
		file.flush();
	}
	file.close();
}
void LOG::write_log(SOCKET client,string username,string path)
{
	auto now = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	std::tm now_tm; // ע�����ﲻ��Ҫ��ʼ����localtime_s ��������

	// ������ֱ�ӵ��ã���Ҫ�� std::
	localtime_r(&now_c, &now_tm);

	std::ostringstream oss;
	oss << now_tm.tm_year + 1900 << "-"
		<< std::setfill('0') << std::setw(2) << now_tm.tm_mon + 1 << "-"
		<< std::setfill('0') << std::setw(2) << now_tm.tm_mday << " "
		<< std::setfill('0') << std::setw(2) << now_tm.tm_hour << ":"
		<< std::setfill('0') << std::setw(2) << now_tm.tm_min << ":"
		<< std::setfill('0') << std::setw(2) << now_tm.tm_sec;
	string time = oss.str();
	string log,IP,ip;
	IP = getip(client, ip);
	log = username + "   " + time +"   "+path+"    "+IP+ '\n';
	block_queue->push(log);
}
void LOG::write_log(SOCKET client, string path)
{
	auto now = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	std::tm now_tm; // ע�����ﲻ��Ҫ��ʼ����localtime_s ��������

	// ������ֱ�ӵ��ã���Ҫ�� std::
	localtime_r(&now_c, &now_tm);

	std::ostringstream oss;
	oss << now_tm.tm_year + 1900 << "-"
		<< std::setfill('0') << std::setw(2) << now_tm.tm_mon + 1 << "-"
		<< std::setfill('0') << std::setw(2) << now_tm.tm_mday << " "
		<< std::setfill('0') << std::setw(2) << now_tm.tm_hour << ":"
		<< std::setfill('0') << std::setw(2) << now_tm.tm_min << ":"
		<< std::setfill('0') << std::setw(2) << now_tm.tm_sec;
	string time = oss.str();
	string log, IP, ip;
	IP = getip(client, ip);
	log = path+"  " + time + "    " + IP + '\n';
	block_queue->push(log);
}