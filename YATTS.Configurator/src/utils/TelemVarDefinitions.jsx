import DescWarning from '@/components/DescWarning';

export default [
  {
    def: {
      id: 'channel',
      name: 'Channel variables',
      desc: 'streamed continuously'
    },
    groups: [
      {
        def: {
          id: 'common',
          name: 'Common'
        },
        variables: [
          {
            id: 'local.scale',
            name: 'Local scale',
            type: 5,
            desc: () => <>
              <p>Scale applied to distance and time to compensate for the scale of the map (e.g. 1s of real time corresponds to <code>local.scale</code> seconds of simulated game time).</p>
              <p>Games which use real 1:1 maps will not provide this channel.</p>
              <p>Type: <code>float</code></p>
              <DescWarning>This is a warning message!</DescWarning>
            </>
          },
          {
            id: 'game.time',
            name: 'Game time',
            type: 3,
            desc: () => <>
              this is just a text literal
            </>
          }
        ]
      },
      {
        def: {
          id: 'truck',
          name: 'Truck'
        },
        variables: [
          {
            id: 'speed',
            name: 'Speed',
            type: 5
          },
          {
            id: 'blinker.left',
            name: 'Left blinker (toggle)',
            type: 1
          }
        ]
      },
      {
        def: {
          id: 'trailer',
          name: 'Trailer'
        },
        variables: [
          {
            id: 'connected',
            name: 'Connected',
            type: 1
          },
          {
            id: 'wheel.speed',
            name: 'Wheel speed',
            type: 5
          }
        ]
      }
    ]
  },
  {
    def: {
      id: 'config',
      name: 'Config variables',
      desc: 'sent only, when a group\'s variable changes'
    },
    groups: [
      {
        def: {
          id: 'truck',
          name: 'Truck'
        },
        variables: [
          {
            id: 'id',
            type: 15
          },
          {
            id: 'registration.plate',
            type: 15
          }
        ]
      }
    ]
  },
  {
    def: {
      id: 'event',
      name: 'Event variables',
      desc: 'sent only, when an event occurs'
    },
    groups: []
  }
]
