import hdmlp

job = hdmlp.Job("/tmp/test",
                128,
                100,
                "uniform",
                True,
                None)
job.setup()




job.destroy()