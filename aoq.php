<?php


error_reporting(E_ALL); 


class Aoq{
    
    private $socket;
    
    public function connect($host='127.0.0.1', $port=8899)
    {
        $this->socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);  
        if ($this->socket < 0) {  
            echo "socket创建失败原因: " . socket_strerror($this->socket) . "\n";
            exit;
        } 
 
        $conn = socket_connect($this->socket, $host, $port);
        return $conn;

    }
    
    
    private function parseArgn($res, $headlen)
    {

        $i=0;
        $start = 4;
        $argn = array();
        while(($start+1) < $headlen)
        {
            $argn[$i++] = (int)substr($res, $start, 6);
            $start+=6;
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
        $i = 0;
        $start = 4;
        $argn = $this->parseArgn($res, $headlen);
        $argv = $this->parseArgv($res, $headlen, $argn);
        return $argv;
    }
    
    private function getResponse()
    {
        $ret = '';
        $out = '';
    
        $out = socket_read($this->socket, 8192);
    
        if($out)
        {
            $ret .= $out; 
            while(strlen($out) == 8192)
            {
            
                $out = socket_read($this->socket, 8192);
            
                $ret .= $out; 
            }
        }
        
        return $ret;
    }
    
    public function status()
    {
        $head = "0501 \n";
        socket_write($this->socket, $head, strlen($head));
        $ret = $this->getResponse();
        $ret = $this->parseResponse($ret);
        return $ret;
    }
    
    public function push($qname, $value)
    {
        if(!$qname || strlen($qname)>=1024)
        {
            return false;
        }
        
        $head = sprintf("1502%06d%06d ", strlen($qname), strlen($value));
        socket_write($this->socket, $head.$qname.$value."\n", strlen($head)+strlen($qname)+strlen($value)+1);
        $ret = $this->getResponse();
        $ret = $this->parseResponse($ret);
        return $ret;
        
    }
    
    public function pop($qname)
    {
        if(!$qname || strlen($qname)>=1024)
        {
            return false;
        }
        
        $head = sprintf("1003%06d ", strlen($qname));
        socket_write($this->socket, $head.$qname."\n", strlen($head)+strlen($qname)+1);
        $ret = $this->getResponse();
        $ret = $this->parseResponse($ret);
        return $ret;
        
    }
    
    public function queues()
    {
        $head = "0504 \n";
        socket_write($this->socket, $head, strlen($head));
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
        
        $head = sprintf("1005%06d ", strlen($qname));
        socket_write($this->socket, $head.$qname."\n", strlen($head)+strlen($qname)+1);
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
        
        $head = sprintf("1006%06d ", strlen($qname));
        socket_write($this->socket, $head.$qname."\n", strlen($head)+strlen($qname)+1);
        $ret = $this->getResponse();
        $ret = $this->parseResponse($ret);
        return $ret;
    }
    
    public function close()
    {
        socket_close($this->socket);
    }
    
    public function __destruct()
    {
        $this->close();
    }
    
}

function mtime()
{
	return microtime(true);
}

$aoq = new Aoq();
$aoq->connect('127.0.0.1', 8899);


$res = $aoq->status();
var_dump($res );

$push_data = str_repeat('ok', 1024);
$res = $aoq->push('test', $push_data);

$res = $aoq->pop('test');
var_dump($res);

$res = $aoq->queues();
var_dump($res);

$res = $aoq->queue('test');
var_dump($res );

$res = $aoq->delqueue('test');
var_dump($res);

$res = $aoq->queues();
var_dump($res);



