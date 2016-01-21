
package com.mogujie.io;

public class NetCheckResult {
	
	public int parse_dns_result;
	public int parse_dns_use_time;

	public int connect_result;
	public int connect_time;

	public int send_req_result;
	public int send_req_bytes;
	public int send_req_time;

	public int recv_rsp_result;
	public int recv_rsp_bytes;

	int error;

	@Override
	public String toString() {
		return "parse_dns_result:  "+parse_dns_result+ ", parse_dns_use_time: "+parse_dns_use_time+" connect_result:"+connect_result
				+" connect_time: "+connect_time;
	}
}