import hdmlp

batch_size = 128
epochs = 10

job = hdmlp.Job("/tmp/test",
                batch_size,
                epochs,
                "uniform",
                True,
                None)
job.setup()

for i in range(batch_size * epochs):
    print(job.get())

job.destroy()