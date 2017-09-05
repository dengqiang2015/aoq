<?php
error_reporting(E_ALL); 
/*
class Aoq{
    
    private $socket;
	const BUFFER_SIZE = 8192;

    public function __construct()
    {
    }

    public function connect($host = '127.0.0.1', $port = 5211)
    {
        $recv_buf_size = $send_buf_size = 16*1024;
        $this->socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
		
        socket_set_option($this->socket, SOL_SOCKET, TCP_NODELAY, 1);  
        socket_set_option($this->socket, SOL_SOCKET, SO_SNDBUF, $send_buf_size);  
        socket_set_option($this->socket, SOL_SOCKET, SO_RCVBUF, $recv_buf_size);  
		
        $result = socket_connect($this->socket, $host, $port);

        if($result === false) {
            echo "socket_connect() failed.\nReason: ({$result}) " . socket_strerror(socket_last_error($this->socket)) . "\n";
            exit;
        }
        return $this->socket;
    }
    
    
    private function parseArgn($res, $headlen)
    {

        $i = 0;
        $start = 4;
        $argn = array();
        while(($start+1) < $headlen)
        {
            $argn[$i++] = (int)substr($res, $start, 6);
            $start += 6;
        }
        return $argn;
    }
    
    private function parseArgv($res, $headlen, $argn)
    {
        $argv = array();
        $i = 0;
        $start = $headlen;
        foreach($argn as $n)
        {
            $val = substr($res, $start, $n);
            $argv[$i++] = array("lenth"=>$n, 'argv'=>$val);
            $start += $n;
        }
        
        return $argv;
    }
    
    private function parseResponse($res)
    {
        $res = rtrim($res, "\n");
        $headlen = (int)substr($res, 0, 2);
        $argn = $this->parseArgn($res, $headlen);
        $argv = $this->parseArgv($res, $headlen, $argn);
        return $argv;
    }
    
    private function getResponse()
    {
		$response = '';
		while($out = socket_read($this->socket, self::BUFFER_SIZE))
		{
			 $response .= $out;
			 if(strlen($out) < self::BUFFER_SIZE)
			 {
				 break;
			 }
		}
		return $response;
    }
    
    public function status()
    {
        $head = "0501 \n";
        $rs = socket_write($this->socket, $head, 6);
        $ret = $this->getResponse();
        $ret = $this->parseResponse($ret);
        return $ret;
    }
    
    public function push($qname, $value)
    {
        if(!$qname || strlen($qname) >= 1024)
        {
            return false;
        }

        $qnlen = strlen($qname);
        $vlen = strlen($value);
        $head = sprintf("1502%06d%06d ", $qnlen, $vlen);
        socket_write($this->socket, $head.$qname.$value."\n", $qnlen+$vlen+18);
        $ret = $this->getResponse();
        $ret = $this->parseResponse($ret);
        return $ret;
        
    }
    
    public function pop($qname)
    {
        if(!$qname || strlen($qname) >= 1024)
        {
            return false;
        }

        $qnlen = strlen($qname); 
        $head = sprintf("1003%06d ", $qnlen);
        socket_write($this->socket, $head.$qname."\n", $qnlen+12);
        $ret = $this->getResponse();
        $ret = $this->parseResponse($ret);
        return $ret;
        
    }


    public function queues()
    {
        $head = "0504 \n";
		socket_write($this->socket, $head, 6);
        $ret = $this->getResponse();
        $ret = $this->parseResponse($ret);
        return $ret;
        
    }
    
    public function queue($qname)
    {
        if(!$qname || strlen($qname)>=1024)
        {
            return false;
        }
        
		$qnlen = strlen($qname);
        $head = sprintf("1005%06d ", $qnlen);
		socket_write($this->socket, $head.$qname."\n", $qnlen+12);
        $ret = $this->getResponse();
        $ret = $this->parseResponse($ret);
        return $ret;
        
    }
    
    public function delqueue($qname)
    {
        if(!$qname || strlen($qname)>=1024)
        {
            return false;
        }
		
        $qnlen = strlen($qname);
        $head = sprintf("1006%06d ", $qnlen);
		socket_write($this->socket, $head.$qname."\n", $qnlen+12);
        $ret = $this->getResponse();
        $ret = $this->parseResponse($ret);
        return $ret;
    }

    public function close()
    {
        socket_close($this->socket);
		$this->socket = NULL;
    }
    
    public function __destruct()
    {
		if($this->socket)
		{
			$this->close();
		}
    }
    
}

function mtime()
{
    list($usec, $sec) = explode(' ', microtime());
    $sec = (string)$sec;
    $sec = substr($sec, -4);
    return ((float)$sec + (float)$usec);
}
*/

$aoq = new Aoq();
$aoq->connect('127.0.0.1', 5211);

/*
$res = $aoq->status();
var_dump($res);
*/

$push_data = str_repeat('ok', 1024);

for($i=0;$i<50000;$i++){
	$res = $aoq->push('test', $push_data);
	$res = $aoq->pop('test');
}

/*
$res = $aoq->queues();
var_dump($res);

$res = $aoq->queue('test');
var_dump($res);

$res = $aoq->delqueue('test');
var_dump($res);

$res = $aoq->queues();
var_dump($res);
*/
