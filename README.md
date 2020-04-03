# Scalability (HW2 for ECE 568 ERSS 2020 Spring)

By Qing Lu, Chixiang Zhang, Yijun Mao

To run server side code, plese type:

```
sudo docker-compose up
```

If docker does not work, plese type:

```
cd scalability
bash runserver.sh
```

to run server code. 

You can adjust the thread strategy (thread per request, thread pool), bucket size, or CPU core number for the client by alternating the values of `THREAD_FORM`, `NUM_BUCKETS` and `NUM_CORES` in `runserver.sh`, respectively, whose instructions are written in `runserver.sh`.

<!-- `s` stands for running server side code.

`server_para2` stands for the thread strategy, whose value 0 means thread per request, and 1 means thread pool.

`server_para3` stands for the size of bucket, whose value 1 refers to 32 bucket size, 2 refers to 128 bucket size, 3 refers to 512 bucket size, and 4 refers to 2048 bucket size. -->

You must run server side code first (keep the server running) before running client side code!

To run client side code, please type:

```
cd testing
bash runclient.sh
```

You can adjust the number of requests, variation type, or bucket size for the client by alternating the values of `NUM_REQUESTS`, `VARIATION_TYPE` and `NUM_BUCKETS` in `runclient.sh`, respectively, whose instructions are written in `runclient.sh`.