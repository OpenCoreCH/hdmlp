import hdmlp

job = hdmlp.Job("/test",
                128,
                "uniform",
                True,
                None)
job.setup()