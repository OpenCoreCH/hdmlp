import hdmlp

batch_size = 128
epochs = 10

path = "/tmp/test"
#path = "/Volumes/Daten/Daten/Datasets/hymenoptera_data/train" # (245 files)

job = hdmlp.Job(path,
                batch_size,
                epochs,
                "uniform",
                True,
                None)
job.setup()
print("Setup complete")

for i in range(batch_size * epochs // 2):
    label, sample = job.get()
    print(label)
    print(sample)


job.destroy()