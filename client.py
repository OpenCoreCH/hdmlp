import hdmlp

job = hdmlp.Job("/tmp/test",
                128,
                100,
                "uniform",
                True,
                None)
job.setup()

for i in range(128):
    print(job.get())

job.destroy()