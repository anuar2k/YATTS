<template>
    <div class="row">
        <h4 class="col p-2 text-center bg-primary text-white">{{ formattedTime }}</h4>
        <input type="file" v-on:change="readFile($event)"/>
    </div>
</template>

<script>
export default {
  name: 'TimeHeader',
  data() {
    return {
      time: new Date()
    }
  },
  computed: {
    formattedTime() {
      const hours = this.time.getHours().toString().padStart(2, '0');
      const minutes = this.time.getMinutes().toString().padStart(2, '0');
      const seconds = this.time.getSeconds().toString().padStart(2, '0');
      return `${hours}:${minutes}:${seconds}`;
    }
  },
  methods: {
    incTimer() {
      this.time = new Date();
    },
    readFile(event) {
      const file = event.target.files[0];
      console.log(file);
      if (file != null) {
        const reader = new FileReader();
        reader.onload = () => {
          console.log(reader.result);
        }
        reader.readAsText(file);
      }
    }
  },
  mounted() {
    setInterval(this.incTimer, 1000);
  }
}
</script>
