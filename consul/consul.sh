consul agent -server -ui -node=server-1 -client=0.0.0.0 -bootstrap-expect=1 -data-dir=/tmp/consul &
sleep 4

consul kv put hazelcast/cluster/map_cluster '{"name": "hello-world", "members": {"name": "hz1", "port": "5701"}}'
consul kv put hazelcast/cluster/queue_cluster '{"name": "q", "members": {"name": "hz", "port": "5701"}}'
consul kv put hazelcast/map map
consul kv put hazelcast/queue queue

wait
